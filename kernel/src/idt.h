#pragma once

#include <stdint.h>

// Initialize the Interrupt Descriptor Table and PIC
void idt_init(void);

// Enable interrupts (STI)
static inline void enable_interrupts(void) {
    asm volatile ("sti");
}

// Disable interrupts (CLI)
static inline void disable_interrupts(void) {
    asm volatile ("cli");
}
