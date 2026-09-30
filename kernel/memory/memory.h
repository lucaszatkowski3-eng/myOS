#pragma once
#include <stdint.h>
#include <stddef.h>

void memory_init(void);
void *kmalloc(size_t size, size_t alignment);
uint64_t memory_total(void);
uint64_t memory_free(void);
int memory_ready(void);
