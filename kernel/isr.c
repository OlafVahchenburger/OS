#include "isr.h"
#include "idt.h"
#include "../drivers/vga.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

static isr_t handlers[256];

/* ---- ISR / IRQ extern stubs (defined in isr.asm) ----------------------- */
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);

extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

/* Remap PIC1 to int 32-39, PIC2 to int 40-47 */
static void pic_remap(void) {
    uint8_t m1 = inb(PIC1_DATA);
    uint8_t m2 = inb(PIC2_DATA);

    outb(PIC1_CMD,  0x11);  /* init command */
    outb(PIC2_CMD,  0x11);
    outb(PIC1_DATA, 0x20);  /* PIC1 vector offset: 32 */
    outb(PIC2_DATA, 0x28);  /* PIC2 vector offset: 40 */
    outb(PIC1_DATA, 0x04);  /* PIC2 is at IRQ2 */
    outb(PIC2_DATA, 0x02);  /* cascade identity */
    outb(PIC1_DATA, 0x01);  /* 8086 mode */
    outb(PIC2_DATA, 0x01);
    outb(PIC1_DATA, m1);    /* restore masks */
    outb(PIC2_DATA, m2);
}

static const char *exc_names[] = {
    "Divide-by-Zero",      "Debug",                "NMI",
    "Breakpoint",          "Overflow",             "Bound Range Exceeded",
    "Invalid Opcode",      "Device Not Available", "Double Fault",
    "Coprocessor Seg OVR", "Invalid TSS",          "Segment Not Present",
    "Stack-Segment Fault", "General Protection",   "Page Fault",
    "Reserved",            "x87 FP Exception",     "Alignment Check",
    "Machine Check",       "SIMD FP Exception",    "Virtualization",
    "Reserved","Reserved","Reserved","Reserved","Reserved",
    "Reserved","Reserved","Reserved","Reserved",
    "Security Exception",  "Reserved"
};

/* Called from isr_common in isr.asm */
void isr_dispatch(registers_t *r) {
    if (handlers[r->int_no]) {
        handlers[r->int_no](r);
        return;
    }
    /* Unhandled exception: print info and halt */
    vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
    vga_print("\n[EXCEPTION] ");
    if (r->int_no < 32) vga_print(exc_names[r->int_no]);
    vga_print(" (int=");
    vga_print_dec(r->int_no);
    vga_print(", err=");
    vga_print_hex(r->err_code);
    vga_print(", eip=");
    vga_print_hex(r->eip);
    vga_print(")\nSystem halted.\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    __asm__ volatile("cli; hlt");
    __builtin_unreachable();
}

/* Called from irq_common in isr.asm */
void irq_dispatch(registers_t *r) {
    /* Send EOI before calling the handler so nested IRQs are possible */
    if (r->int_no >= 40) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);

    if (handlers[r->int_no])
        handlers[r->int_no](r);
}

void isr_register_handler(uint8_t n, isr_t h) {
    handlers[n] = h;
}

void isr_init(void) {
    pic_remap();

    /* CPU exceptions 0-31 */
    idt_set_gate( 0, (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate( 1, (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate( 2, (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate( 3, (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate( 4, (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate( 5, (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate( 6, (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate( 7, (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate( 8, (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate( 9, (uint32_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    /* Hardware IRQs 0-15 (remapped to 32-47) */
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    __asm__ volatile("sti");
}
