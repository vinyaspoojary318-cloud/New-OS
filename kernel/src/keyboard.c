#include "keyboard.h"
#include "console.h"
#include "idt.h"

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Simple scancode to ASCII (US QWERTY, no shift handling for now)
static const char scancode_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', 0,
};

static char key_buffer[256];
static volatile size_t key_read = 0;
static volatile size_t key_write = 0;

void keyboard_init(void) {
    // Nothing special needed for basic PS/2
    key_read = 0;
    key_write = 0;
}

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    // Only handle key press (ignore key release which has bit 7 set)
    if (scancode & 0x80) {
        return;
    }

    char c = 0;
    if (scancode < 128) {
        c = scancode_ascii[scancode];
    }

    if (c != 0) {
        size_t next = (key_write + 1) % 256;
        if (next != key_read) { // buffer not full
            key_buffer[key_write] = c;
            key_write = next;
        }
    }
}

bool keyboard_has_char(void) {
    return key_read != key_write;
}

char keyboard_getchar(void) {
    while (!keyboard_has_char()) {
        asm volatile ("hlt"); // wait for interrupt
    }
    char c = key_buffer[key_read];
    key_read = (key_read + 1) % 256;
    return c;
}
