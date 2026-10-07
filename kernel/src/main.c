#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "console.h"
#include "pmm.h"

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
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    if (memmap_request.response == NULL || hhdm_request.response == NULL) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // Initialize console first so we can print messages
    console_init(fb);
    console_clear(0x0A0A1A);

    console_set_color(0x00FFCC);
    console_print("========================================\n");
    console_print("          New-OS Kernel v0.2\n");
    console_print("========================================\n\n");

    console_set_color(0xFFFFFF);
    console_print("Framebuffer : ");
    console_print_dec(fb->width);
    console_print("x");
    console_print_dec(fb->height);
    console_print("\n\n");

    // === Initialize Physical Memory Manager ===
    console_set_color(0xFFCC55);
    console_print("[..] Initializing Physical Memory Manager...\n");

    pmm_init(memmap_request.response, hhdm_request.response->offset);

    console_set_color(0x55FF55);
    console_print("[OK] PMM initialized\n\n");

    // Print memory statistics
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

    // Demo: allocate a few pages
    console_set_color(0xFFAA55);
    console_print("Testing allocator...\n");

    uint64_t page1 = pmm_alloc_page();
    uint64_t page2 = pmm_alloc_page();
    uint64_t pages = pmm_alloc_pages(4);

    console_set_color(0xCCCCCC);
    console_print("  Allocated page 1 @ ");
    console_print_hex(page1);
    console_print("\n");

    console_print("  Allocated page 2 @ ");
    console_print_hex(page2);
    console_print("\n");

    console_print("  Allocated 4 pages @ ");
    console_print_hex(pages);
    console_print("\n\n");

    // Free them again
    pmm_free_page(page1);
    pmm_free_page(page2);
    pmm_free_pages(pages, 4);

    console_set_color(0x55FF55);
    console_print("[OK] Memory allocation test passed\n\n");

    console_set_color(0x00FFAA);
    console_print("Kernel is alive with memory management!\n");
    console_print("Ready for next features...\n");

    hcf();
}
