#ifndef HEAP_H
#define HEAP_H

#include "../include/types.h"

void  heap_init(void);
void *kmalloc(size_t size);
void  kfree(void *ptr);
void *krealloc(void *ptr, size_t new_size);

#endif
