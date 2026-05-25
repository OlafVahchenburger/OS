#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../mm/pmm.h"
#include "../mm/heap.h"
#include "../fs/fs.h"
#include "../shell/shell.h"
#include "../include/types.h"

#define MULTIBOOT_MAGIC 0x2BADB002U

/* Minimal multiboot info structure (flags field selects which fields exist) */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;   /* KiB below 1 MiB */
    uint32_t mem_upper;   /* KiB above 1 MiB */
    /* … more fields we don't need */
} __attribute__((packed)) mb_info_t;

static void print_banner(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("  ____              ___  ____  \n");
    vga_print(" |  _ \\ __ _ _ __ / _ \\/ ___| \n");
    vga_print(" | |_) / _` | '_ | | | \\___ \\ \n");
    vga_print(" |  _ | (_| | | | | |_| |___) |\n");
    vga_print(" |_| \\_\\__,_|_| |_|\\___/|____/ \n");
    vga_set_color(VGA_DARK_GREY, VGA_BLACK);
    vga_print("              v0.1  –  hobby OS\n\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
}

static void ok(const char *msg) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_print("  [ OK ] ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_print(msg);
    vga_putchar('\n');
}

void kernel_main(uint32_t magic, mb_info_t *mbi) {
    vga_init();
    print_banner();

    /* ---- GDT ------------------------------------------------------------ */
    gdt_init();
    ok("GDT loaded");

    /* ---- IDT ------------------------------------------------------------ */
    idt_init();
    ok("IDT loaded");

    /* ---- ISR / IRQ / PIC ------------------------------------------------ */
    isr_init();          /* also calls sti() */
    ok("Interrupts enabled");

    /* ---- Physical memory manager ---------------------------------------- */
    uint32_t mem_bytes;
    if (magic == MULTIBOOT_MAGIC && (mbi->flags & 1)) {
        mem_bytes = (mbi->mem_upper + 1024u) * 1024u;
    } else {
        mem_bytes = 32u * 1024u * 1024u;   /* assume 32 MiB */
    }
    pmm_init(mem_bytes);

    /* ---- Heap ------------------------------------------------------------ */
    heap_init();
    ok("Memory management ready");
    vga_print("         RAM: ");
    vga_print_dec(mem_bytes / (1024 * 1024));
    vga_print(" MiB  |  heap: 1 MiB  |  free pages: ");
    vga_print_dec(pmm_free_count());
    vga_putchar('\n');

    /* ---- Keyboard driver ------------------------------------------------ */
    keyboard_init();
    ok("PS/2 keyboard driver loaded");

    /* ---- Filesystem ----------------------------------------------------- */
    fs_init();
    ok("In-memory filesystem ready");

    vga_putchar('\n');

    /* ---- Hand off to the shell ------------------------------------------ */
    shell_run();

    /* Should never reach here */
    __asm__ volatile("cli; hlt");
    __builtin_unreachable();
}
