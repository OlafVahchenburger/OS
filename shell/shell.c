#include "shell.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../fs/fs.h"
#include "../mm/pmm.h"
#include "../mm/heap.h"
#include "../libc/string.h"
#include "../include/types.h"

#define CMD_MAX  256
#define ARGS_MAX 16

/* ---- command parser ----------------------------------------------------- */

static int parse(char *line, char **argv) {
    int argc = 0;
    char *p = line;
    while (*p && argc < ARGS_MAX) {
        while (*p == ' ') p++;
        if (!*p) break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p) *p++ = '\0';
    }
    return argc;
}

/* ---- prompt ------------------------------------------------------------- */

static void print_prompt(void) {
    vga_set_color(VGA_LIGHT_GREEN,  VGA_BLACK);
    vga_print("root@ranos");
    vga_set_color(VGA_LIGHT_GREY,   VGA_BLACK);
    vga_putchar(':');
    vga_set_color(VGA_LIGHT_BLUE,   VGA_BLACK);
    vga_print(fs_getcwd());
    vga_set_color(VGA_WHITE,        VGA_BLACK);
    vga_print("# ");
    vga_set_color(VGA_LIGHT_GREY,   VGA_BLACK);
}

/* ---- built-in commands -------------------------------------------------- */

static void do_help(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("RanOS built-in commands\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_print("  help                  show this message\n");
    vga_print("  clear                 clear the screen\n");
    vga_print("  echo [args...]        print arguments\n");
    vga_print("  ls   [path]           list directory\n");
    vga_print("  cat  <file>           print file contents\n");
    vga_print("  mkdir <dir>           create directory\n");
    vga_print("  touch <file>          create empty file\n");
    vga_print("  rm    <path>          remove file\n");
    vga_print("  write <file> <text>   write text to file\n");
    vga_print("  cd    [dir]           change directory\n");
    vga_print("  pwd                   print working directory\n");
    vga_print("  meminfo               memory statistics\n");
    vga_print("  about                 OS information\n");
    vga_print("  reboot                reboot the system\n");
    vga_print("  halt                  halt the system\n");
}

static void do_clear(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_clear();
}

static void do_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) vga_putchar(' ');
        vga_print(argv[i]);
    }
    vga_putchar('\n');
}

static void do_ls(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : fs_getcwd();
    char      names[64][FS_MAX_NAME];
    fs_type_t types[64];
    int n = fs_list(path, names, types, 64);
    if (n < 0) {
        vga_print("ls: ");
        vga_print(path);
        vga_print(": no such directory\n");
        return;
    }
    if (n == 0) { vga_print("(empty)\n"); return; }
    for (int i = 0; i < n; i++) {
        if (types[i] == FS_DIR) {
            vga_set_color(VGA_LIGHT_BLUE, VGA_BLACK);
            vga_print(names[i]);
            vga_putchar('/');
        } else {
            vga_set_color(VGA_WHITE, VGA_BLACK);
            vga_print(names[i]);
        }
        vga_print("  ");
    }
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_putchar('\n');
}

static void do_cat(int argc, char **argv) {
    if (argc < 2) { vga_print("usage: cat <file>\n"); return; }
    uint8_t buf[FS_MAX_DATA + 1];
    int sz = fs_read(argv[1], buf, FS_MAX_DATA);
    if (sz < 0) {
        vga_print("cat: ");
        vga_print(argv[1]);
        vga_print(": no such file\n");
        return;
    }
    buf[sz] = '\0';
    vga_print((char *)buf);
    if (sz > 0 && buf[sz - 1] != '\n') vga_putchar('\n');
}

static void do_mkdir(int argc, char **argv) {
    if (argc < 2) { vga_print("usage: mkdir <dir>\n"); return; }
    if (fs_mkdir(argv[1]) < 0) {
        vga_print("mkdir: cannot create '");
        vga_print(argv[1]);
        vga_print("'\n");
    }
}

static void do_touch(int argc, char **argv) {
    if (argc < 2) { vga_print("usage: touch <file>\n"); return; }
    if (!fs_exists(argv[1]) && fs_create(argv[1], FS_FILE) < 0) {
        vga_print("touch: cannot create '");
        vga_print(argv[1]);
        vga_print("'\n");
    }
}

static void do_rm(int argc, char **argv) {
    if (argc < 2) { vga_print("usage: rm <path>\n"); return; }
    if (fs_delete(argv[1]) < 0) {
        vga_print("rm: cannot remove '");
        vga_print(argv[1]);
        vga_print("'\n");
    }
}

static void do_write(int argc, char **argv) {
    if (argc < 3) { vga_print("usage: write <file> <text...>\n"); return; }
    /* Reassemble words into a single string */
    char buf[CMD_MAX];
    buf[0] = '\0';
    for (int i = 2; i < argc; i++) {
        if (i > 2) strcat(buf, " ");
        strncat(buf, argv[i], sizeof(buf) - strlen(buf) - 2);
    }
    strcat(buf, "\n");
    if (fs_write(argv[1], (uint8_t *)buf, (uint32_t)strlen(buf)) < 0) {
        vga_print("write: failed writing to '");
        vga_print(argv[1]);
        vga_print("'\n");
    }
}

static void do_cd(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : "/";
    if (!fs_exists(path)) {
        vga_print("cd: ");
        vga_print(path);
        vga_print(": no such directory\n");
        return;
    }
    fs_chdir(path);
}

static void do_pwd(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_print(fs_getcwd());
    vga_putchar('\n');
}

static void do_meminfo(int argc, char **argv) {
    (void)argc; (void)argv;
    uint32_t free_pg  = pmm_free_count();
    uint32_t total_pg = pmm_total_count();
    vga_print("Physical memory:\n");
    vga_print("  Total : ");
    vga_print_dec(total_pg * 4);
    vga_print(" KiB (");
    vga_print_dec(total_pg);
    vga_print(" pages)\n");
    vga_print("  Free  : ");
    vga_print_dec(free_pg * 4);
    vga_print(" KiB (");
    vga_print_dec(free_pg);
    vga_print(" pages)\n");
    vga_print("  Used  : ");
    vga_print_dec((total_pg - free_pg) * 4);
    vga_print(" KiB\n");
}

static void do_about(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_print("\n  RanOS v0.1\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_print("  A hobby OS written from scratch\n");
    vga_print("  Arch : x86 (32-bit protected mode)\n");
    vga_print("  Boot : GRUB multiboot or custom MBR bootloader\n");
    vga_print("  FS   : in-memory flat filesystem\n");
    vga_print("  MM   : bitmap PMM + explicit free-list heap\n\n");
}

static void do_reboot(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_print("Rebooting...\n");
    /* Pulse the PS/2 reset line via keyboard controller */
    __asm__ volatile(
        "mov $0xFE, %%al\n"
        "out %%al, $0x64\n"
        : : : "eax"
    );
    __asm__ volatile("cli; hlt");
}

static void do_halt(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_print("System halted. Goodbye!\n");
    __asm__ volatile("cli; hlt");
}

/* ---- dispatch table ----------------------------------------------------- */

typedef void (*cmd_fn)(int, char **);
typedef struct { const char *name; cmd_fn fn; } cmd_t;

static const cmd_t cmds[] = {
    { "help",    do_help    },
    { "clear",   do_clear   },
    { "echo",    do_echo    },
    { "ls",      do_ls      },
    { "cat",     do_cat     },
    { "mkdir",   do_mkdir   },
    { "touch",   do_touch   },
    { "rm",      do_rm      },
    { "write",   do_write   },
    { "cd",      do_cd      },
    { "pwd",     do_pwd     },
    { "meminfo", do_meminfo },
    { "about",   do_about   },
    { "reboot",  do_reboot  },
    { "halt",    do_halt    },
    { NULL, NULL }
};

static void execute(char *line) {
    char *argv[ARGS_MAX];
    int   argc = parse(line, argv);
    if (!argc) return;

    for (int i = 0; cmds[i].name; i++) {
        if (strcmp(argv[0], cmds[i].name) == 0) {
            cmds[i].fn(argc, argv);
            return;
        }
    }
    vga_print(argv[0]);
    vga_print(": command not found\n");
}

/* ---- shell entry point -------------------------------------------------- */

void shell_run(void) {
    /* Print message of the day */
    uint8_t motd[FS_MAX_DATA + 1];
    int n = fs_read("/etc/motd", motd, FS_MAX_DATA);
    if (n > 0) {
        motd[n] = '\0';
        vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
        vga_print((char *)motd);
        vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    }
    vga_print("Type 'help' for available commands.\n\n");

    static char buf[CMD_MAX];

    for (;;) {
        print_prompt();
        int len = 0;

        for (;;) {
            char c = keyboard_getchar();
            if (c == '\n') {
                vga_putchar('\n');
                buf[len] = '\0';
                execute(buf);
                break;
            } else if (c == '\b') {
                if (len > 0) { len--; vga_putchar('\b'); }
            } else if (len < CMD_MAX - 1) {
                buf[len++] = c;
                vga_putchar(c);
            }
        }
    }
}
