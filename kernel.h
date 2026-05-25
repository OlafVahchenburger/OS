#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

/* ── VGA text mode ─────────────────────────────────────────────────────────── */

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_BUFFER ((volatile uint16_t *)0xB8000)

typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color_t;

void vga_init(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putchar(char c);
void vga_puts(const char *str);

/* ── IDT ───────────────────────────────────────────────────────────────────── */

#define IDT_ENTRIES 256

/*
 * Each 8-byte IDT gate descriptor (32-bit protected mode format).
 * type_attr = 0x8E for a present, ring-0, 32-bit interrupt gate.
 */
typedef struct __attribute__((packed)) {
    uint16_t offset_low;   /* handler address [15:0]  */
    uint16_t selector;     /* code segment selector   */
    uint8_t  zero;         /* always 0                */
    uint8_t  type_attr;    /* gate type + DPL + P bit */
    uint16_t offset_high;  /* handler address [31:16] */
} idt_entry_t;

/* 6-byte structure loaded by LIDT. */
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t flags);
void idt_init(void);

/* ── Kernel entry ──────────────────────────────────────────────────────────── */

void kernel_main(void);

#endif /* KERNEL_H */
