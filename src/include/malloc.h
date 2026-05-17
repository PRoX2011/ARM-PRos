#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

void malloc_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
