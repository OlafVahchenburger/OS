[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, msg
.print_loop:
    lodsb
    test al, al
    jz .halt
    mov ah, 0x0E        ; BIOS teletype output
    mov bh, 0x00        ; page 0
    mov bl, 0x07        ; white on black
    int 0x10
    jmp .print_loop

.halt:
    cli
    hlt
    jmp .halt           ; catch spurious NMI wakeups

msg:
    db 'Welcome to RanOS!', 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xAA55
