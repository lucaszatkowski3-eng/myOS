#include "paging.h"
#define ENTRIES 512
static uint64_t pml4[ENTRIES] __attribute__((aligned(4096)));
static uint64_t pdpt[ENTRIES] __attribute__((aligned(4096)));
static uint64_t pd[ENTRIES] __attribute__((aligned(4096)));
static int ready;
int paging_init(void){
 for(int i=0;i<ENTRIES;i++){pml4[i]=0;pdpt[i]=0;pd[i]=0;}
 pml4[0]=(uint64_t)(uintptr_t)pdpt|3; pdpt[0]=(uint64_t)(uintptr_t)pd|3;
 for(int i=0;i<ENTRIES;i++) pd[i]=((uint64_t)i*0x200000ull)|0x83;
 __asm__ volatile("mov %0,%%cr3"::"r"((uint64_t)(uintptr_t)pml4):"memory");
 ready=1; return 0;
}
int paging_ready(void){return ready;}
uint64_t paging_kernel_pml4(void){return (uint64_t)(uintptr_t)pml4;}
