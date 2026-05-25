#include "keyboard.h"
#include "../kernel/isr.h"

#define KB_DATA   0x60
#define KB_STATUS 0x64
#define KB_BUF    256

/* US QWERTY scancode set 1 -> ASCII (unshifted) */
static const char sc_normal[128] = {
    0,   27,  '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q', 'w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a', 's','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/',
    0,   '*', 0,  ' ', 0,
    0,0,0,0,0,0,0,0,0,0, /* F1-F10 */
    0,0,                 /* NumLock, ScrollLock */
    '7','8','9','-',
    '4','5','6','+',
    '1','2','3','0','.', 0,0,0,
    0,0                  /* F11, F12 */
};

static const char sc_shift[128] = {
    0,   27,  '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q', 'W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,   'A', 'S','D','F','G','H','J','K','L',':','"','~',
    0,   '|', 'Z','X','C','V','B','N','M','<','>','?',
    0,   '*', 0,  ' ', 0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,
    '7','8','9','-',
    '4','5','6','+',
    '1','2','3','0','.', 0,0,0,
    0,0
};

static char    kbd_buf[KB_BUF];
static volatile int kbd_head = 0;
static volatile int kbd_tail = 0;
static bool    shift_down = false;
static bool    caps_lock  = false;

static inline uint8_t inb(uint16_t port) {
    uint8_t v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static void kbd_irq_handler(registers_t *regs) {
    (void)regs;
    uint8_t sc = inb(KB_DATA);

    /* Shift press/release */
    if (sc == 0x2A || sc == 0x36) { shift_down = true;  return; }
    if (sc == 0xAA || sc == 0xB6) { shift_down = false; return; }
    /* Caps Lock toggle */
    if (sc == 0x3A) { caps_lock = !caps_lock; return; }

    if (sc & 0x80) return; /* other key-release, ignore */
    if (sc >= 128)  return;

    char c;
    char lo = sc_normal[sc];
    char hi = sc_shift[sc];

    if (lo >= 'a' && lo <= 'z') {
        /* Letter: caps lock XOR shift determines case */
        c = (caps_lock ^ shift_down) ? hi : lo;
    } else {
        c = shift_down ? hi : lo;
    }

    if (!c) return;

    int next = (kbd_head + 1) % KB_BUF;
    if (next != kbd_tail) {
        kbd_buf[kbd_head] = c;
        kbd_head = next;
    }
}

void keyboard_init(void) {
    isr_register_handler(IRQ1, kbd_irq_handler);
}

bool keyboard_has_input(void) {
    return kbd_head != kbd_tail;
}

char keyboard_getchar(void) {
    while (!keyboard_has_input())
        __asm__ volatile("hlt");
    char c = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KB_BUF;
    return c;
}
