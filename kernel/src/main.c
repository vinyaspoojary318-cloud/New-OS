#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "console.h"

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // Initialize the text console
    console_init(fb);

    // Clear screen with dark background
    console_clear(0x0A0A1A);

    // === Boot messages ===
    console_set_color(0x00FFCC);
    console_print("========================================\n");
    console_print("          New-OS Kernel v0.1\n");
    console_print("========================================\n\n");

    console_set_color(0xFFFFFF);
    console_print("Framebuffer initialized\n");

    console_set_color(0xAAAAAA);
    console_print("  Resolution : ");
    console_print_dec(fb->width);
    console_print(" x ");
    console_print_dec(fb->height);
    console_print("\n");

    console_print("  Pitch      : ");
    console_print_dec(fb->pitch);
    console_print("\n");

    console_print("  BPP        : ");
    console_print_dec(fb->bpp);
    console_print("\n\n");

    console_set_color(0x55FF55);
    console_print("[OK] Console ready\n");
    console_print("[OK] Framebuffer ready\n\n");

    console_set_color(0xFFCC55);
    console_print("Next goals:\n");
    console_set_color(0xCCCCCC);
    console_print("  1. Memory management\n");
    console_print("  2. Interrupts + keyboard\n");
    console_print("  3. Simple filesystem\n");
    console_print("  4. Natural language commands\n\n");

    console_set_color(0x00FFAA);
    console_print("Kernel is alive. Waiting...\n");

    // Halt forever
    hcf();
}
