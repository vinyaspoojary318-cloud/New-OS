#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

// Use base revision 3 for compatibility
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

// Request a framebuffer
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

// Helper: put a pixel (assumes 32-bit RGB)
static void put_pixel(struct limine_framebuffer *fb, size_t x, size_t y, uint32_t color) {
    if (x >= fb->width || y >= fb->height) return;
    volatile uint32_t *fb_ptr = fb->address;
    fb_ptr[y * (fb->pitch / 4) + x] = color;
}

// Draw a filled rectangle
static void fill_rect(struct limine_framebuffer *fb, size_t x, size_t y, size_t w, size_t h, uint32_t color) {
    for (size_t dy = 0; dy < h; dy++) {
        for (size_t dx = 0; dx < w; dx++) {
            put_pixel(fb, x + dx, y + dy, color);
        }
    }
}

// Draw a border (hollow rectangle)
static void draw_border(struct limine_framebuffer *fb, size_t x, size_t y, size_t w, size_t h, size_t thickness, uint32_t color) {
    // Top
    fill_rect(fb, x, y, w, thickness, color);
    // Bottom
    fill_rect(fb, x, y + h - thickness, w, thickness, color);
    // Left
    fill_rect(fb, x, y, thickness, h, color);
    // Right
    fill_rect(fb, x + w - thickness, y, thickness, h, color);
}

// Kernel entry point
void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // === Colorful background gradient ===
    for (size_t y = 0; y < fb->height; y++) {
        for (size_t x = 0; x < fb->width; x++) {
            uint8_t r = (uint8_t)(x * 255 / fb->width);
            uint8_t g = (uint8_t)(y * 255 / fb->height);
            uint8_t b = (uint8_t)(128 + (x + y) % 128);
            uint32_t color = (r << 16) | (g << 8) | b;
            put_pixel(fb, x, y, color);
        }
    }

    // === Outer thick white border ===
    draw_border(fb, 20, 20, fb->width - 40, fb->height - 40, 8, 0xFFFFFF);

    // === Inner cyan border ===
    draw_border(fb, 40, 40, fb->width - 80, fb->height - 80, 4, 0x00FFFF);

    // === Accent rectangles (top bar style) ===
    fill_rect(fb, 60, 60, fb->width - 120, 40, 0x222244);   // dark bar
    fill_rect(fb, 60, 60, 8, 40, 0x00FF88);                 // green accent

    // === Decorative colored squares in corners ===
    fill_rect(fb, 70, 110, 50, 50, 0xFF5555);   // red
    fill_rect(fb, 130, 110, 50, 50, 0x55FF55);  // green
    fill_rect(fb, 190, 110, 50, 50, 0x5555FF);  // blue
    fill_rect(fb, 250, 110, 50, 50, 0xFFFF55);  // yellow

    // === Simple "NEW-OS" style blocks (pixel art like) ===
    // Just some nice horizontal lines as decoration
    for (int i = 0; i < 6; i++) {
        uint32_t colors[] = {0xFF0066, 0xFF6600, 0xFFCC00, 0x00CC66, 0x0066FF, 0x6600FF};
        fill_rect(fb, 70, 180 + i * 12, 300, 8, colors[i]);
    }

    // === Final thin white frame around the content area ===
    draw_border(fb, 55, 55, fb->width - 110, fb->height - 110, 2, 0xFFFFFF);

    // Halt forever - kernel is alive and looking good!
    hcf();
}
