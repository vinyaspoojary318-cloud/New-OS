#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

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

static void put_pixel(struct limine_framebuffer *fb, size_t x, size_t y, uint32_t color) {
    if (x >= fb->width || y >= fb->height) return;
    volatile uint32_t *ptr = fb->address;
    ptr[y * (fb->pitch / 4) + x] = color;
}

static void fill_rect(struct limine_framebuffer *fb, size_t x, size_t y, size_t w, size_t h, uint32_t color) {
    for (size_t dy = 0; dy < h; dy++) {
        for (size_t dx = 0; dx < w; dx++) {
            put_pixel(fb, x + dx, y + dy, color);
        }
    }
}

static void draw_border(struct limine_framebuffer *fb, size_t x, size_t y, size_t w, size_t h, size_t thickness, uint32_t color) {
    fill_rect(fb, x, y, w, thickness, color);                     // top
    fill_rect(fb, x, y + h - thickness, w, thickness, color);     // bottom
    fill_rect(fb, x, y, thickness, h, color);                     // left
    fill_rect(fb, x + w - thickness, y, thickness, h, color);     // right
}

// Very simple 5x7 bitmap font for a few letters (uppercase only)
static const uint8_t font_N[7] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
static const uint8_t font_E[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
static const uint8_t font_W[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};
static const uint8_t font_O[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
static const uint8_t font_S[7] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
static const uint8_t font_dash[7] = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};

static void draw_char(struct limine_framebuffer *fb, size_t x, size_t y, const uint8_t *glyph, uint32_t color, int scale) {
    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            if (glyph[row] & (1 << (4 - col))) {
                fill_rect(fb, x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

static void draw_title(struct limine_framebuffer *fb, size_t cx, size_t cy) {
    int scale = 6;
    int char_w = 5 * scale + 8;
    int total_w = 6 * char_w;
    size_t start_x = cx - total_w / 2;
    size_t start_y = cy - (7 * scale) / 2;

    // Soft shadow
    draw_char(fb, start_x + 3, start_y + 3, font_N, 0x111122, scale);
    draw_char(fb, start_x + char_w + 3, start_y + 3, font_E, 0x111122, scale);
    draw_char(fb, start_x + 2*char_w + 3, start_y + 3, font_W, 0x111122, scale);
    draw_char(fb, start_x + 3*char_w + 3, start_y + 3, font_dash, 0x111122, scale);
    draw_char(fb, start_x + 4*char_w + 3, start_y + 3, font_O, 0x111122, scale);
    draw_char(fb, start_x + 5*char_w + 3, start_y + 3, font_S, 0x111122, scale);

    // Main white title
    draw_char(fb, start_x, start_y, font_N, 0xFFFFFF, scale);
    draw_char(fb, start_x + char_w, start_y, font_E, 0xFFFFFF, scale);
    draw_char(fb, start_x + 2*char_w, start_y, font_W, 0xFFFFFF, scale);
    draw_char(fb, start_x + 3*char_w, start_y, font_dash, 0x00FFCC, scale);
    draw_char(fb, start_x + 4*char_w, start_y, font_O, 0xFFFFFF, scale);
    draw_char(fb, start_x + 5*char_w, start_y, font_S, 0xFFFFFF, scale);
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) hcf();

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // === Smooth dark-to-purple gradient background ===
    for (size_t y = 0; y < fb->height; y++) {
        for (size_t x = 0; x < fb->width; x++) {
            uint8_t r = (uint8_t)(20 + (x * 40 / fb->width));
            uint8_t g = (uint8_t)(10 + (y * 30 / fb->height));
            uint8_t b = (uint8_t)(40 + (x + y) * 80 / (fb->width + fb->height));
            put_pixel(fb, x, y, (r << 16) | (g << 8) | b);
        }
    }

    // === Outer thick border ===
    draw_border(fb, 16, 16, fb->width - 32, fb->height - 32, 6, 0xFFFFFF);

    // === Inner cyan border ===
    draw_border(fb, 30, 30, fb->width - 60, fb->height - 60, 3, 0x00E5FF);

    // === Centered title box ===
    size_t box_w = 520;
    size_t box_h = 140;
    size_t box_x = (fb->width - box_w) / 2;
    size_t box_y = (fb->height - box_h) / 2 - 40;

    // Box background
    fill_rect(fb, box_x, box_y, box_w, box_h, 0x0A0A1A);

    // Box borders
    draw_border(fb, box_x, box_y, box_w, box_h, 4, 0xFFFFFF);
    draw_border(fb, box_x + 8, box_y + 8, box_w - 16, box_h - 16, 2, 0x00FFAA);

    // Title text
    draw_title(fb, fb->width / 2, box_y + box_h / 2);

    // === Decorative bottom accent bar ===
    size_t bar_y = box_y + box_h + 40;
    fill_rect(fb, box_x, bar_y, box_w, 12, 0x111122);
    fill_rect(fb, box_x, bar_y, 8, 12, 0x00FF88);
    fill_rect(fb, box_x + box_w - 8, bar_y, 8, 12, 0xFF55AA);

    // === Four colorful accent squares under the bar ===
    size_t sq = 36;
    size_t gap = 20;
    size_t start_sq = box_x + (box_w - (4 * sq + 3 * gap)) / 2;
    uint32_t colors[] = {0xFF5555, 0x55FF55, 0x5599FF, 0xFFCC33};
    for (int i = 0; i < 4; i++) {
        fill_rect(fb, start_sq + i * (sq + gap), bar_y + 30, sq, sq, colors[i]);
        draw_border(fb, start_sq + i * (sq + gap), bar_y + 30, sq, sq, 2, 0xFFFFFF);
    }

    // === Small status text area (simple lines) ===
    size_t status_y = bar_y + 90;
    fill_rect(fb, box_x + 40, status_y, box_w - 80, 4, 0x334455);
    fill_rect(fb, box_x + 40, status_y + 14, box_w - 120, 4, 0x334455);

    hcf();
}
