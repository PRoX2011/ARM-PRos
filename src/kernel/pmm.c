#include <pmm.h>
#include <string.h>
#include <log.h>
#include <drivers/mailbox.h>

extern uint8_t __end;

static uint8_t *bitmap;
static uint64_t total_pages;
static uint64_t bitmap_size;

void pmm_init(void) {
    volatile uint32_t __attribute__((aligned(16))) mbox[8];
    mbox[0] = 8 * 4;
    mbox[1] = 0;
    mbox[2] = 0x00010005; // Get ARM memory
    mbox[3] = 8;
    mbox[4] = 0;
    mbox[5] = 0;
    mbox[6] = 0;
    mbox[7] = 0;

    uint64_t mem_size = 1024 * 1024 * 1024;
    if (mbox_call(mbox)) {
        mem_size = mbox[6];
    }

    total_pages = mem_size / PAGE_SIZE;
    bitmap_size = total_pages / 8;
    
    bitmap = &__end;
    memset(bitmap, 0xFF, bitmap_size);
    
    uint64_t kernel_end = (uint64_t)bitmap + bitmap_size;
    uint64_t first_free_page = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    for (uint64_t addr = first_free_page; addr < mem_size; addr += PAGE_SIZE) {
        pmm_free_page((void*)addr);
    }
}

void *pmm_alloc_page(void) {
    return pmm_alloc_pages(1);
}

void *pmm_alloc_pages(size_t count) {
    uint64_t found = 0;
    for (uint64_t i = 0; i < total_pages; i++) {
        uint64_t page = i;
        if (!(bitmap[page / 8] & (1 << (page % 8)))) {
            found++;
            if (found == count) {
                uint64_t start_page = i - count + 1;
                for (uint64_t j = start_page; j <= i; j++) {
                    bitmap[j / 8] |= (1 << (j % 8));
                }
                return (void*)(start_page * PAGE_SIZE);
            }
        } else {
            found = 0;
        }
    }
    return NULL;
}

void pmm_free_page(void *ptr) {
    pmm_free_pages(ptr, 1);
}

void pmm_free_pages(void *ptr, size_t count) {
    uint64_t addr = (uint64_t)ptr;
    uint64_t start_page = addr / PAGE_SIZE;
    for (uint64_t i = 0; i < count; i++) {
        uint64_t page = start_page + i;
        bitmap[page / 8] &= ~(1 << (page % 8));
    }
}
