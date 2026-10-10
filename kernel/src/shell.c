#include "shell.h"
#include "console.h"
#include "keyboard.h"
#include "pmm.h"
#include <stddef.h>
#include <stdint.h>

#define INPUT_MAX 128

static char input_buffer[INPUT_MAX];
static size_t input_len = 0;

// Simple string helpers
static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static int str_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
}

static void print_prompt(void) {
    console_set_color(0x00FFAA);
    console_print("new-os> ");
    console_set_color(0xFFFFFF);
}

static void cmd_help(void) {
    console_set_color(0x55FF55);
    console_print("Available commands:\n");
    console_set_color(0xCCCCCC);
    console_print("  help     - Show this help\n");
    console_print("  clear    - Clear the screen\n");
    console_print("  mem      - Show memory information\n");
    console_print("  echo ... - Print text\n");
    console_print("  about    - About New-OS\n");
    console_print("\n");
    console_set_color(0xFFCC55);
    console_print("Natural language support coming soon...\n");
    console_print("Try typing things like 'create folder' later.\n\n");
}

static void cmd_clear(void) {
    console_clear(0x0A0A1A);
}

static void cmd_mem(void) {
    console_set_color(0xAAAAFF);
    console_print("Memory Information:\n");
    console_set_color(0xCCCCCC);

    console_print("  Total usable : ");
    console_print_dec(pmm_get_total_memory() / 1024 / 1024);
    console_print(" MiB\n");

    console_print("  Free         : ");
    console_print_dec(pmm_get_free_memory() / 1024 / 1024);
    console_print(" MiB\n");

    console_print("  Used         : ");
    console_print_dec(pmm_get_used_memory() / 1024 / 1024);
    console_print(" MiB\n\n");
}

static void cmd_about(void) {
    console_set_color(0x00FFCC);
    console_print("New-OS v0.4\n");
    console_set_color(0xCCCCCC);
    console_print("A new operating system designed so ordinary people\n");
    console_print("can control the computer with natural language.\n\n");
    console_print("Current features:\n");
    console_print("  - 64-bit kernel\n");
    console_print("  - Framebuffer console\n");
    console_print("  - Physical memory manager\n");
    console_print("  - Interrupts + keyboard\n");
    console_print("  - Simple interactive shell\n\n");
}

static void cmd_echo(const char *args) {
    if (args && *args) {
        console_print(args);
        console_print("\n");
    } else {
        console_print("\n");
    }
}

static void process_command(char *line) {
    // Skip leading spaces
    while (*line == ' ') line++;

    if (*line == 0) return; // empty line

    if (str_eq(line, "help")) {
        cmd_help();
    } else if (str_eq(line, "clear")) {
        cmd_clear();
    } else if (str_eq(line, "mem") || str_eq(line, "memory")) {
        cmd_mem();
    } else if (str_eq(line, "about")) {
        cmd_about();
    } else if (str_starts_with(line, "echo ")) {
        cmd_echo(line + 5);
    } else if (str_eq(line, "echo")) {
        cmd_echo("");
    } else {
        console_set_color(0xFF5555);
        console_print("Unknown command: ");
        console_print(line);
        console_print("\n");
        console_set_color(0xAAAAAA);
        console_print("Type 'help' for available commands.\n\n");
    }
}

void shell_run(void) {
    console_set_color(0x00FFCC);
    console_print("========================================\n");
    console_print("       New-OS Interactive Shell\n");
    console_print("========================================\n\n");
    console_set_color(0xCCCCCC);
    console_print("Type 'help' to see available commands.\n\n");

    print_prompt();

    for (;;) {
        if (!keyboard_has_char()) {
            asm volatile ("hlt");
            continue;
        }

        char c = keyboard_getchar();

        if (c == '\n') {
            console_print("\n");
            input_buffer[input_len] = 0;
            process_command(input_buffer);
            input_len = 0;
            print_prompt();
        } else if (c == '\b') {
            // Backspace
            if (input_len > 0) {
                input_len--;
                // Simple visual backspace: print backspace + space + backspace
                // (our console doesn't have real cursor control yet)
                console_print("\b \b");
            }
        } else if (c >= 32 && c < 127) {
            // Printable character
            if (input_len < INPUT_MAX - 1) {
                input_buffer[input_len++] = c;
                char buf[2] = {c, 0};
                console_print(buf);
            }
        }
    }
}
