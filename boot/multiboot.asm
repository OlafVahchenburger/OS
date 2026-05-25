; =============================================================================
; multiboot.asm  –  GRUB Multiboot v1 kernel entry point
;
; GRUB loads this ELF kernel, verifies the multiboot header, then jumps
; to _start with:
;   eax = 0x2BADB002  (multiboot magic)
;   ebx = physical address of multiboot_info struct
; =============================================================================

[BITS 32]

; ---- Multiboot header constants --------------------------------------------
MULTIBOOT_MAGIC    equ 0x1BADB002
MULTIBOOT_FLAGS    equ 0x00000003   ; request memory map + module alignment
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

; ---- Multiboot header (must be in first 8 KiB of the kernel image) --------
section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

; ---- Bootstrap stack (16 KiB) ----------------------------------------------
section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

; ---- Entry point -----------------------------------------------------------
section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top      ; set up stack pointer
    push ebx                ; arg 2: multiboot_info *
    push eax                ; arg 1: multiboot magic
    call kernel_main
    ; kernel_main should never return
.halt:
    cli
    hlt
    jmp .halt

; Non-executable stack note
section .note.GNU-stack noalloc noexec nowrite progbits
