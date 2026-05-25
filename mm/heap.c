#include "heap.h"
#include "../libc/string.h"

/*
 * Simple explicit free-list allocator.
 * A static 1 MiB arena lives in BSS (zeroed by GRUB).
 * Blocks carry a header; adjacent free blocks are coalesced on kfree.
 */

#define HEAP_SIZE (1024u * 1024u)   /* 1 MiB */
#define ALIGN8(x) (((x) + 7u) & ~7u)

typedef struct block {
    size_t        size;   /* payload bytes (not including header) */
    bool          used;
    struct block *next;
    struct block *prev;
} block_t;

static uint8_t  heap_arena[HEAP_SIZE] __attribute__((aligned(16)));
static block_t *heap_head;

void heap_init(void) {
    heap_head       = (block_t *)heap_arena;
    heap_head->size = HEAP_SIZE - sizeof(block_t);
    heap_head->used = false;
    heap_head->next = NULL;
    heap_head->prev = NULL;
}

void *kmalloc(size_t size) {
    if (!size) return NULL;
    size = ALIGN8(size);

    for (block_t *b = heap_head; b; b = b->next) {
        if (b->used || b->size < size) continue;

        /* Split if leftover would fit a header + at least 8 bytes */
        if (b->size >= size + sizeof(block_t) + 8) {
            block_t *nb = (block_t *)((uint8_t *)b + sizeof(block_t) + size);
            nb->size    = b->size - size - sizeof(block_t);
            nb->used    = false;
            nb->next    = b->next;
            nb->prev    = b;
            if (b->next) b->next->prev = nb;
            b->next     = nb;
            b->size     = size;
        }
        b->used = true;
        return (void *)((uint8_t *)b + sizeof(block_t));
    }
    return NULL;   /* out of heap */
}

void kfree(void *ptr) {
    if (!ptr) return;
    block_t *b = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    b->used = false;

    /* Coalesce with next */
    if (b->next && !b->next->used) {
        b->size += sizeof(block_t) + b->next->size;
        b->next  = b->next->next;
        if (b->next) b->next->prev = b;
    }
    /* Coalesce with previous */
    if (b->prev && !b->prev->used) {
        b->prev->size += sizeof(block_t) + b->size;
        b->prev->next  = b->next;
        if (b->next) b->next->prev = b->prev;
    }
}

void *krealloc(void *ptr, size_t new_size) {
    if (!ptr)     return kmalloc(new_size);
    if (!new_size) { kfree(ptr); return NULL; }

    block_t *b = (block_t *)((uint8_t *)ptr - sizeof(block_t));
    if (b->size >= ALIGN8(new_size)) return ptr;

    void *np = kmalloc(new_size);
    if (np) { memcpy(np, ptr, b->size); kfree(ptr); }
    return np;
}
