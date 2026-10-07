#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

// Initialize the physical memory manager using Limine memmap
void pmm_init(struct limine_memmap_response *memmap, uint64_t hhdm_offset);

// Allocate one 4 KiB page. Returns physical address, or 0 on failure.
uint64_t pmm_alloc_page(void);

// Free a previously allocated page (physical address)
void pmm_free_page(uint64_t phys_addr);

// Allocate multiple contiguous pages. Returns starting physical address, or 0.
uint64_t pmm_alloc_pages(size_t count);

// Free multiple contiguous pages
void pmm_free_pages(uint64_t phys_addr, size_t count);

// Get statistics
uint64_t pmm_get_total_memory(void);      // in bytes
uint64_t pmm_get_free_memory(void);       // in bytes
uint64_t pmm_get_used_memory(void);       // in bytes

// Convert physical <-> virtual using HHDM
void *pmm_phys_to_virt(uint64_t phys);
uint64_t pmm_virt_to_phys(void *virt);
