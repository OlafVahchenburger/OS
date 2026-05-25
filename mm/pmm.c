#include "pmm.h"
#include "../libc/string.h"

/*
 * Bitmap physical memory manager.
 * Pages below PMM_BASE are reserved (BIOS, video, kernel).
 * Bitmap itself lives at PMM_BITMAP_ADDR.
 */

#define PMM_BITMAP_ADDR 0x100000U   /* 1 MiB  – just above conventional mem */
#define PMM_BASE        0x200000U   /* 2 MiB  – first page we hand out       */

static uint32_t *bitmap;
static uint32_t  total_pages;
static uint32_t  free_pages;

static void bm_set(uint32_t bit)   { bitmap[bit / 32] |=  (1u << (bit % 32)); }
static void bm_clear(uint32_t bit) { bitmap[bit / 32] &= ~(1u << (bit % 32)); }
static int  bm_test(uint32_t bit)  { return (bitmap[bit / 32] >> (bit % 32)) & 1; }

void pmm_init(uint32_t mem_bytes) {
    bitmap      = (uint32_t *)PMM_BITMAP_ADDR;
    total_pages = mem_bytes / PAGE_SIZE;
    free_pages  = 0;

    /* Mark every page as used */
    memset(bitmap, 0xFF, (total_pages / 8) + 1);

    /* Free pages starting at PMM_BASE */
    uint32_t first = PMM_BASE / PAGE_SIZE;
    for (uint32_t i = first; i < total_pages; i++) {
        bm_clear(i);
        free_pages++;
    }
}

void *pmm_alloc_page(void) {
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!bm_test(i)) {
            bm_set(i);
            free_pages--;
            return (void *)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void pmm_free_page(void *addr) {
    uint32_t page = (uint32_t)addr / PAGE_SIZE;
    if (page < total_pages && bm_test(page)) {
        bm_clear(page);
        free_pages++;
    }
}

uint32_t pmm_free_count(void)  { return free_pages;  }
uint32_t pmm_total_count(void) { return total_pages; }
