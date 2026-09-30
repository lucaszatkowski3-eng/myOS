#pragma once
#include <stdint.h>
int paging_init(void);
int paging_ready(void);
uint64_t paging_kernel_pml4(void);
