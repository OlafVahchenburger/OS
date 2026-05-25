; =============================================================================
; isr.asm  –  ISR/IRQ stubs + GDT/IDT flush helpers
; =============================================================================

[BITS 32]

global gdt_flush
global idt_flush

global isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7
global isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

global irq0,  irq1,  irq2,  irq3,  irq4,  irq5,  irq6,  irq7
global irq8,  irq9,  irq10, irq11, irq12, irq13, irq14, irq15

extern isr_dispatch
extern irq_dispatch

; --------------------------------------------------------------------------
; GDT / IDT loaders
; --------------------------------------------------------------------------

gdt_flush:
    mov eax, [esp+4]
    lgdt [eax]
    ; Reload segment registers with kernel data selector (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; Far jump to flush instruction pipeline and reload CS (kernel code 0x08)
    jmp 0x08:.flush
.flush:
    ret

idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

; --------------------------------------------------------------------------
; ISR macros
; --------------------------------------------------------------------------

; CPU does NOT push an error code for these exceptions
%macro ISR_NOERRCODE 1
isr%1:
    push dword 0        ; dummy error code
    push dword %1       ; interrupt number
    jmp isr_common
%endmacro

; CPU DOES push an error code for these exceptions
%macro ISR_ERRCODE 1
isr%1:
    push dword %1       ; interrupt number (error code already on stack)
    jmp isr_common
%endmacro

%macro IRQ_STUB 2
irq%1:
    push dword 0        ; dummy error code
    push dword %2       ; remapped interrupt number (32+n)
    jmp irq_common
%endmacro

; --------------------------------------------------------------------------
; CPU exception stubs (int 0-31)
; --------------------------------------------------------------------------

ISR_NOERRCODE  0    ; Divide-by-zero
ISR_NOERRCODE  1    ; Debug
ISR_NOERRCODE  2    ; NMI
ISR_NOERRCODE  3    ; Breakpoint
ISR_NOERRCODE  4    ; Overflow
ISR_NOERRCODE  5    ; Bound Range Exceeded
ISR_NOERRCODE  6    ; Invalid Opcode
ISR_NOERRCODE  7    ; Device Not Available
ISR_ERRCODE    8    ; Double Fault           (error code pushed by CPU)
ISR_NOERRCODE  9    ; Coprocessor Seg Overrun
ISR_ERRCODE   10    ; Invalid TSS
ISR_ERRCODE   11    ; Segment Not Present
ISR_ERRCODE   12    ; Stack-Segment Fault
ISR_ERRCODE   13    ; General Protection Fault
ISR_ERRCODE   14    ; Page Fault
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 FP Exception
ISR_ERRCODE   17    ; Alignment Check
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; SIMD FP Exception
ISR_NOERRCODE 20    ; Virtualization Exception
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; --------------------------------------------------------------------------
; IRQ stubs (int 32-47 after PIC remap)
; --------------------------------------------------------------------------

IRQ_STUB  0, 32
IRQ_STUB  1, 33
IRQ_STUB  2, 34
IRQ_STUB  3, 35
IRQ_STUB  4, 36
IRQ_STUB  5, 37
IRQ_STUB  6, 38
IRQ_STUB  7, 39
IRQ_STUB  8, 40
IRQ_STUB  9, 41
IRQ_STUB 10, 42
IRQ_STUB 11, 43
IRQ_STUB 12, 44
IRQ_STUB 13, 45
IRQ_STUB 14, 46
IRQ_STUB 15, 47

; --------------------------------------------------------------------------
; Common ISR stub
; Stack layout on entry here:
;   [esp+0] int_no, [esp+4] err_code,
;   then CPU-pushed: eip, cs, eflags [, useresp, ss on ring change]
; --------------------------------------------------------------------------

isr_common:
    pusha                   ; pushes eax,ecx,edx,ebx,esp,ebp,esi,edi
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10            ; kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp                ; pointer to registers_t for the C handler
    call isr_dispatch
    add esp, 4              ; pop the pointer argument

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8              ; discard int_no + err_code
    iret

; --------------------------------------------------------------------------
; Common IRQ stub  (same layout, just calls irq_dispatch)
; --------------------------------------------------------------------------

irq_common:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_dispatch
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

; Non-executable stack note
section .note.GNU-stack noalloc noexec nowrite progbits
