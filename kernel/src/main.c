#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

// Set the base revision to the latest recommended.
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Request a framebuffer from the bootloader
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Halt and catch fire
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

// Kernel entry point
void kmain(void) {
    // Ensure the bootloader actually understands our base revision
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // Draw a simple white diagonal line so we can visually confirm the kernel is running
    for (size_t i = 0; i < 200; i++) {
        volatile uint32_t *fb_ptr = fb->address;
        size_t offset = i * (fb->pitch / 4) + i;
        if (offset < (fb->height * fb->pitch / 4)) {
            fb_ptr[offset] = 0xFFFFFF;   // white pixel
        }
    }

    // Halt forever (kernel is alive)
    hcf();
}
