#include "gdt.h"

/* Each GDT entry is 8 bytes */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;     /* present, ring, type bits */
    uint8_t  gran;       /* 4-bit flags + 4-bit limit high */
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_entry gdt[5];
static struct gdt_ptr   gdtp;

/* Defined in kernel/isr.asm */
extern void gdt_flush(uint32_t gdtp_addr);

static void set_entry(int i, uint32_t base, uint32_t limit,
                       uint8_t access, uint8_t gran) {
    gdt[i].base_low  = (uint16_t)(base & 0xFFFF);
    gdt[i].base_mid  = (uint8_t)((base >> 16) & 0xFF);
    gdt[i].base_high = (uint8_t)((base >> 24) & 0xFF);
    gdt[i].limit_low = (uint16_t)(limit & 0xFFFF);
    gdt[i].gran      = (uint8_t)(((limit >> 16) & 0x0F) | (gran & 0xF0));
    gdt[i].access    = access;
}

void gdt_init(void) {
    gdtp.limit = (uint16_t)(sizeof(gdt) - 1);
    gdtp.base  = (uint32_t)&gdt;

    set_entry(0, 0, 0,          0x00, 0x00); /* null */
    set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* kernel code  ring 0 */
    set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* kernel data  ring 0 */
    set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* user   code  ring 3 */
    set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* user   data  ring 3 */

    gdt_flush((uint32_t)&gdtp);
}
