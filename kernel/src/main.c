#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "console.h"
#include "pmm.h"
#include "idt.h"
#include "keyboard.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) hcf();

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1 ||
        memmap_request.response == NULL ||
        hhdm_request.response == NULL) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    console_init(fb);
    console_clear(0x0A0A1A);

    console_set_color(0x00FFCC);
    console_print("========================================\n");
    console_print("          New-OS Kernel v0.3\n");
    console_print("========================================\n\n");

    // Memory Manager
    console_set_color(0xFFCC55);
    console_print("[..] Initializing PMM...\n");
    pmm_init(memmap_request.response, hhdm_request.response->offset);
    console_set_color(0x55FF55);
    console_print("[OK] Physical Memory Manager ready\n");
    console_set_color(0xCCCCCC);
    console_print("     Free memory: ");
    console_print_dec(pmm_get_free_memory() / 1024 / 1024);
    console_print(" MiB\n\n");

    // Interrupts
    console_set_color(0xFFCC55);
    console_print("[..] Setting up IDT and PIC...\n");
    idt_init();
    console_set_color(0x55FF55);
    console_print("[OK] Interrupts ready\n\n");

    // Keyboard
    console_set_color(0xFFCC55);
    console_print("[..] Initializing keyboard...\n");
    keyboard_init();
    console_set_color(0x55FF55);
    console_print("[OK] Keyboard ready\n\n");

    // Enable interrupts
    enable_interrupts();

    console_set_color(0x00FFAA);
    console_print("System ready! Type something:\n\n");
    console_set_color(0xFFFFFF);
    console_print("> ");

    // Simple echo loop
    for (;;) {
        if (keyboard_has_char()) {
            char c = keyboard_getchar();

            if (c == '\n') {
                console_print("\n> ");
            } else if (c == '\b') {
                // Simple backspace handling could be added later
                console_print("^");
            } else {
                char buf[2] = {c, 0};
                console_print(buf);
            }
        } else {
            asm volatile ("hlt");
        }
    }
}
