; =============================================================================
; RanOS – Stage-1 MBR Bootloader (512 bytes)
; BIOS loads this at 0x7C00 in 16-bit real mode.
; Prints "Welcome to RanOS", loads the kernel from disk, switches to
; 32-bit protected mode, and jumps to the kernel at 0x10000.
; =============================================================================

[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; stack grows down from 0x7C00
    sti

    mov [boot_drive], dl    ; BIOS passes boot drive in DL

    ; Print welcome message
    mov si, msg_welcome
    call print_str

    ; ---- Load kernel from disk into 0x1000:0x0000 (= 0x10000 physical) ----
    mov ax, 0x1000
    mov es, ax
    xor bx, bx              ; offset 0

    mov ah, 0x02            ; BIOS Read Sectors
    mov al, 32              ; read 32 sectors (16 KiB)
    mov ch, 0               ; cylinder 0
    mov cl, 2               ; start at sector 2 (sector 1 = this code)
    mov dh, 0               ; head 0
    mov dl, [boot_drive]
    int 0x13
    jc  disk_err

    mov si, msg_loaded
    call print_str

    ; ---- Enter 32-bit protected mode ---------------------------------------
    cli
    lgdt [gdt_desc]

    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    jmp 0x08:pm_entry       ; far jump: CS = kernel code selector

; ---- Subroutines -----------------------------------------------------------

print_str:
    pusha
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz   .done
    int  0x10
    jmp  .loop
.done:
    popa
    ret

disk_err:
    mov si, msg_err
    call print_str
.halt: cli
    hlt
    jmp .halt

; ---- GDT -------------------------------------------------------------------

gdt_start:
    dq 0                        ; null descriptor
gdt_code:
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xCF, 0x00  ; ring-0 code, 4K, 32-bit
gdt_data:
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00  ; ring-0 data, 4K, 32-bit
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; ---- Data ------------------------------------------------------------------

boot_drive: db 0
msg_welcome: db "Welcome to RanOS", 13, 10, 0
msg_loaded:  db "Kernel loaded, entering PM...", 13, 10, 0
msg_err:     db "Disk error!", 13, 10, 0

; ---- 32-bit protected-mode entry (still in same 512-byte sector) -----------

[BITS 32]
pm_entry:
    mov ax, 0x10            ; kernel data selector
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x9FC00        ; stack just below EBDA
    jmp 0x10000             ; jump to kernel loaded at 0x10000

; ---- Boot signature --------------------------------------------------------
[BITS 16]
times 510 - ($ - $$) db 0
dw 0xAA55
