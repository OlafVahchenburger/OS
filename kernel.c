#include "kernel.h"

/* ══════════════════════════════════════════════════════════════════════════════
 * VGA text mode (80×25, buffer at physical address 0xB8000)
 * Each cell is two bytes: [character][attribute]
 * Attribute byte: high nibble = background, low nibble = foreground.
 * ══════════════════════════════════════════════════════════════════════════════ */

static size_t  vga_row;
static size_t  vga_col;
static uint8_t vga_attr;

static inline uint16_t vga_entry(char c, uint8_t attr)
{
    return (uint16_t)(uint8_t)c | ((uint16_t)attr << 8);
}

void vga_set_color(uint8_t fg, uint8_t bg)
{
    vga_attr = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void vga_init(void)
{
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_row = 0;
    vga_col = 0;

    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', vga_attr);
}

/* Scroll the screen up one line and clear the bottom row. */
static void vga_scroll(void)
{
    for (size_t y = 1; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            VGA_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_BUFFER[y * VGA_WIDTH + x];

    for (size_t x = 0; x < VGA_WIDTH; x++)
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_attr);

    vga_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c)
{
    if (c == '\n') {
        vga_col = 0;
        if (++vga_row == VGA_HEIGHT)
            vga_scroll();
        return;
    }

    VGA_BUFFER[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, vga_attr);

    if (++vga_col == VGA_WIDTH) {
        vga_col = 0;
        if (++vga_row == VGA_HEIGHT)
            vga_scroll();
    }
}

void vga_puts(const char *str)
{
    while (*str)
        vga_putchar(*str++);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * x86 I/O port helpers
 * ══════════════════════════════════════════════════════════════════════════════ */

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * 8259A PIC remapping
 *
 * The BIOS leaves the PIC with IRQ0-7 mapped to INT 0x08-0x0F, which
 * collides with CPU exception vectors. Remap master to 0x20 and slave to
 * 0x28 so hardware interrupts don't alias CPU faults.
 * ══════════════════════════════════════════════════════════════════════════════ */

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

static void pic_remap(void)
{
    /* Save existing interrupt masks. */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* ICW1: begin initialisation sequence (cascade mode). */
    outb(PIC1_CMD,  0x11);
    outb(PIC2_CMD,  0x11);

    /* ICW2: vector offsets — master at 0x20, slave at 0x28. */
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    /* ICW3: tell master there is a slave on IRQ2; tell slave its cascade ID. */
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    /* ICW4: 8086 / x86 mode. */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    /* Restore saved masks. */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * IDT — Interrupt Descriptor Table
 * ══════════════════════════════════════════════════════════════════════════════ */

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;

void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t flags)
{
    idt[num].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[num].selector    = selector;
    idt[num].zero        = 0;
    idt[num].type_attr   = flags;
    idt[num].offset_high = (uint16_t)((handler >> 16) & 0xFFFF);
}

/*
 * Minimal ISR stub: save and restore the general-purpose register set, then
 * return from interrupt. All 256 vectors point here until a caller installs
 * a real handler via idt_set_gate().
 */
static void __attribute__((naked)) default_isr(void)
{
    __asm__ volatile(
        "pusha\n\t"
        "popa\n\t"
        "iret\n\t"
    );
}

void idt_init(void)
{
    pic_remap();

    /* 0x08 = kernel code segment selector set by the GDT in the bootloader. */
    for (int i = 0; i < IDT_ENTRIES; i++)
        idt_set_gate((uint8_t)i, (uint32_t)default_isr, 0x08, 0x8E);

    idt_ptr.limit = (uint16_t)(sizeof(idt) - 1);
    idt_ptr.base  = (uint32_t)&idt;

    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

/* ══════════════════════════════════════════════════════════════════════════════
 * Kernel entry point — called by the bootloader / linker entry symbol.
 * ══════════════════════════════════════════════════════════════════════════════ */

void kernel_main(void)
{
    vga_init();
    idt_init();

    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_puts("RanOS Kernel loaded!\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    /* Enable interrupts now that the IDT is in place. */
    __asm__ volatile("sti");

    /* Idle loop — the kernel does nothing further until an IRQ fires. */
    for (;;)
        __asm__ volatile("hlt");
}
