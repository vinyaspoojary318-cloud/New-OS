#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

// Initialize the console with the framebuffer
void console_init(struct limine_framebuffer *fb);

// Clear the entire screen with a color
void console_clear(uint32_t color);

// Set text color (RGB)
void console_set_color(uint32_t fg);

// Print a string at the current cursor position
void console_print(const char *str);

// Print a string at a specific position (in character cells)
void console_print_at(size_t col, size_t row, const char *str);

// Move cursor to a position
void console_set_cursor(size_t col, size_t row);

// Print a newline
void console_newline(void);

// Simple number printing helpers
void console_print_hex(uint64_t value);
void console_print_dec(uint64_t value);
