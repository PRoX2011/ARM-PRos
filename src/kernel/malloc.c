#include <malloc.h>
#include <pmm.h>
#include <stdint.h>

typedef struct block {
    size_t size;
    struct block *next;
    int free;
} block_t;

static block_t *free_list = NULL;

void malloc_init(void) {
    free_list = (block_t*)pmm_alloc_page();
    if (!free_list) return;
    free_list->size = PAGE_SIZE - sizeof(block_t);
    free_list->next = NULL;
    free_list->free = 1;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;
    size = (size + 7) & ~7;
    block_t *curr = free_list;
    while (curr) {
        if (curr->free && curr->size >= size) {
            if (curr->size > size + sizeof(block_t) + 8) {
                block_t *new_block = (block_t*)((uint8_t*)curr + sizeof(block_t) + size);
                new_block->size = curr->size - size - sizeof(block_t);
                new_block->free = 1;
                new_block->next = curr->next;
                curr->size = size;
                curr->next = new_block;
            }
            curr->free = 0;
            return (void*)(curr + 1);
        }
        curr = curr->next;
    }
    size_t pages = (size + sizeof(block_t) + PAGE_SIZE - 1) / PAGE_SIZE;
    block_t *new_pages = (block_t*)pmm_alloc_pages(pages);
    if (!new_pages) return NULL;
    new_pages->size = pages * PAGE_SIZE - sizeof(block_t);
    new_pages->free = 0;
    new_pages->next = free_list;
    free_list = new_pages;
    if (new_pages->size > size + sizeof(block_t) + 8) {
        block_t *split = (block_t*)((uint8_t*)new_pages + sizeof(block_t) + size);
        split->size = new_pages->size - size - sizeof(block_t);
        split->free = 1;
        split->next = new_pages->next;
        new_pages->size = size;
        new_pages->next = split;
    }
    return (void*)(new_pages + 1);
}

void kfree(void *ptr) {
    if (!ptr) return;
    block_t *block = (block_t*)ptr - 1;
    block->free = 1;

    block_t *curr = free_list;
    while (curr) {
        if (curr->free && curr->next && curr->next->free) {
            if ((uint8_t*)curr + sizeof(block_t) + curr->size == (uint8_t*)curr->next) {
                curr->size += sizeof(block_t) + curr->next->size;
                curr->next = curr->next->next;
                continue;
            }
        }
        curr = curr->next;
    }
}
