/*
 * Linker script for RanOS kernel (GRUB multiboot, loaded at 1 MiB)
 */

ENTRY(_start)

SECTIONS {
    /* Kernel is mapped at 1 MiB physical */
    . = 1M;

    /* Multiboot header must appear first (within first 8 KiB) */
    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata*)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
    }
}
