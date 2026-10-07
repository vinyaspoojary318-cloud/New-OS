#pragma once

#include <stdint.h>
#include <stdbool.h>

// Initialize keyboard driver
void keyboard_init(void);

// Called from IRQ1 handler
void keyboard_handler(void);

// Returns true if a character is available
bool keyboard_has_char(void);

// Get next character (blocks if none available in simple version)
char keyboard_getchar(void);
