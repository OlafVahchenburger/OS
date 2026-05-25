#include "idt.h"

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  reserved;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   idtp;

/* Defined in kernel/isr.asm */
extern void idt_flush(uint32_t idtp_addr);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt[num].base_high = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].sel       = sel;
    idt[num].reserved  = 0;
    idt[num].flags     = flags;
}

void idt_init(void) {
    idtp.limit = (uint16_t)(sizeof(idt) - 1);
    idtp.base  = (uint32_t)&idt;

    /* Zero all gates; isr_init() will fill the real ones in */
    for (int i = 0; i < 256; i++)
        idt_set_gate((uint8_t)i, 0, 0, 0);

    idt_flush((uint32_t)&idtp);
}
