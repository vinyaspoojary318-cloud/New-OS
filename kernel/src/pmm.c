#include "pmm.h"
#include "console.h"

#define PAGE_SIZE 4096ULL

// Simple bitmap allocator
// We place the bitmap in a static buffer for simplicity (supports up to ~128 GiB)
#define BITMAP_MAX_PAGES (128ULL * 1024 * 1024 / 4)   // 128 GiB worth of pages
static uint8_t page_bitmap[BITMAP_MAX_PAGES / 8];

static uint64_t total_pages = 0;
static uint64_t free_pages = 0;
static uint64_t highest_page = 0;
static uint64_t hhdm = 0;

static inline void bitmap_set(uint64_t page) {
    page_bitmap[page / 8] |= (1 << (page % 8));
}

static inline void bitmap_clear(uint64_t page) {
    page_bitmap[page / 8] &= ~(1 << (page % 8));
}

static inline bool bitmap_test(uint64_t page) {
    return page_bitmap[page / 8] & (1 << (page % 8));
}

void *pmm_phys_to_virt(uint64_t phys) {
    return (void *)(phys + hhdm);
}

uint64_t pmm_virt_to_phys(void *virt) {
    return (uint64_t)virt - hhdm;
}

void pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset) {
    hhdm = hhdm_offset;

    // First pass: find highest usable address and mark everything as used
    for (size_t i = 0; i < sizeof(page_bitmap); i++) {
        page_bitmap[i] = 0xFF; // all used by default
    }

    highest_page = 0;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        // We only care about usable memory
        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        uint64_t start = entry->base;
        uint64_t end = entry->base + entry->length;

        // Align to page boundaries
        start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        end &= ~(PAGE_SIZE - 1);

        if (end <= start) continue;

        uint64_t start_page = start / PAGE_SIZE;
        uint64_t end_page = end / PAGE_SIZE;

        if (end_page > highest_page) {
            highest_page = end_page;
        }

        // Mark these pages as free
        for (uint64_t p = start_page; p < end_page && p < BITMAP_MAX_PAGES; p++) {
            bitmap_clear(p);
            free_pages++;
            total_pages++;
        }
    }

    // Reserve the very first few pages (null page etc. is already non-usable usually)
    // Also make sure page 0 is never allocated
    if (!bitmap_test(0)) {
        bitmap_set(0);
        free_pages--;
    }
}

uint64_t pmm_alloc_page(void) {
    for (uint64_t p = 1; p < highest_page && p < BITMAP_MAX_PAGES; p++) {
        if (!bitmap_test(p)) {
            bitmap_set(p);
            free_pages--;
            return p * PAGE_SIZE;
        }
    }
    return 0; // out of memory
}

void pmm_free_page(uint64_t phys_addr) {
    if (phys_addr == 0) return;
    uint64_t page = phys_addr / PAGE_SIZE;
    if (page >= BITMAP_MAX_PAGES) return;

    if (bitmap_test(page)) {
        bitmap_clear(page);
        free_pages++;
    }
}

uint64_t pmm_alloc_pages(size_t count) {
    if (count == 0) return 0;
    if (count == 1) return pmm_alloc_page();

    for (uint64_t start = 1; start + count <= highest_page && start + count <= BITMAP_MAX_PAGES; start++) {
        bool ok = true;
        for (size_t i = 0; i < count; i++) {
            if (bitmap_test(start + i)) {
                ok = false;
                start += i; // skip ahead
                break;
            }
        }
        if (ok) {
            for (size_t i = 0; i < count; i++) {
                bitmap_set(start + i);
            }
            free_pages -= count;
            return start * PAGE_SIZE;
        }
    }
    return 0;
}

void pmm_free_pages(uint64_t phys_addr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        pmm_free_page(phys_addr + i * PAGE_SIZE);
    }
}

uint64_t pmm_get_total_memory(void) {
    return total_pages * PAGE_SIZE;
}

uint64_t pmm_get_free_memory(void) {
    return free_pages * PAGE_SIZE;
}

uint64_t pmm_get_used_memory(void) {
    return (total_pages - free_pages) * PAGE_SIZE;
}
