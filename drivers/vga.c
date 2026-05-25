#include "vga.h"

#define VGA_MEMORY   ((uint16_t *)0xB8000)
#define VGA_CTRL_REG 0x3D4
#define VGA_DATA_REG 0x3D5

static uint16_t *vga_buf  = (uint16_t *)0xB8000;
static uint8_t   vga_row  = 0;
static uint8_t   vga_col  = 0;
static uint8_t   vga_attr = 0;   /* fg | (bg << 4) */

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static uint16_t make_entry(char c, uint8_t attr) {
    return (uint16_t)(unsigned char)c | ((uint16_t)attr << 8);
}

static void hw_cursor_update(void) {
    uint16_t pos = (uint16_t)(vga_row * VGA_WIDTH + vga_col);
    outb(VGA_CTRL_REG, 0x0F);
    outb(VGA_DATA_REG, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_REG, 0x0E);
    outb(VGA_DATA_REG, (uint8_t)((pos >> 8) & 0xFF));
}

static void scroll_up(void) {
    for (int row = 1; row < VGA_HEIGHT; row++)
        for (int col = 0; col < VGA_WIDTH; col++)
            vga_buf[(row - 1) * VGA_WIDTH + col] = vga_buf[row * VGA_WIDTH + col];
    for (int col = 0; col < VGA_WIDTH; col++)
        vga_buf[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = make_entry(' ', vga_attr);
    vga_row = VGA_HEIGHT - 1;
}

void vga_init(void) {
    vga_attr = (uint8_t)(VGA_LIGHT_GREY | (VGA_BLACK << 4));
    vga_clear();
}

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga_buf[i] = make_entry(' ', vga_attr);
    vga_row = 0;
    vga_col = 0;
    hw_cursor_update();
}

void vga_putchar(char c) {
    switch (c) {
    case '\n':
        vga_col = 0;
        if (++vga_row >= VGA_HEIGHT) scroll_up();
        break;
    case '\r':
        vga_col = 0;
        break;
    case '\b':
        if (vga_col > 0) {
            vga_col--;
            vga_buf[vga_row * VGA_WIDTH + vga_col] = make_entry(' ', vga_attr);
        }
        break;
    case '\t':
        vga_col = (uint8_t)((vga_col + 8) & ~7u);
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            if (++vga_row >= VGA_HEIGHT) scroll_up();
        }
        break;
    default:
        vga_buf[vga_row * VGA_WIDTH + vga_col] = make_entry(c, vga_attr);
        if (++vga_col >= VGA_WIDTH) {
            vga_col = 0;
            if (++vga_row >= VGA_HEIGHT) scroll_up();
        }
        break;
    }
    hw_cursor_update();
}

void vga_print(const char *str) {
    while (*str) vga_putchar(*str++);
}

void vga_print_hex(uint32_t val) {
    static const char hex[] = "0123456789ABCDEF";
    vga_print("0x");
    for (int i = 28; i >= 0; i -= 4)
        vga_putchar(hex[(val >> i) & 0xF]);
}

void vga_print_dec(uint32_t val) {
    char buf[12];
    int  i = 0;
    if (val == 0) { vga_putchar('0'); return; }
    while (val) { buf[i++] = (char)('0' + val % 10); val /= 10; }
    while (i--) vga_putchar(buf[i]);
}

void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_attr = (uint8_t)((int)fg | ((int)bg << 4));
}

void vga_get_cursor(uint8_t *x, uint8_t *y) {
    *x = vga_col;
    *y = vga_row;
}

void vga_set_cursor(uint8_t x, uint8_t y) {
    vga_col = x;
    vga_row = y;
    hw_cursor_update();
}
