#ifndef ISR_H
#define ISR_H

#include "../include/types.h"

/* Register state pushed by our ISR stubs (matches isr.asm layout) */
typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed)) registers_t;

typedef void (*isr_t)(registers_t *);

void isr_init(void);
void isr_register_handler(uint8_t n, isr_t handler);

/* IRQ numbers after PIC remapping (IRQ0 -> int 32, etc.) */
#define IRQ0  32
#define IRQ1  33
#define IRQ2  34
#define IRQ3  35
#define IRQ4  36
#define IRQ5  37
#define IRQ6  38
#define IRQ7  39
#define IRQ8  40
#define IRQ9  41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

#endif
