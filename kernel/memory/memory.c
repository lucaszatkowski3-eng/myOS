#include "memory.h"
#include "../../src/limine.h"

#define PAGE_SIZE 4096ull
#define MAX_RANGES 128

struct range { uint64_t base, length; };
static struct range usable[MAX_RANGES];
static uint32_t usable_count;
static uint64_t total_mem;
static uint64_t free_mem;
static uint8_t kernel_heap[1024 * 1024] __attribute__((aligned(4096)));
static uint64_t heap_cursor;
static uint64_t heap_end;
static int ready;

static uint64_t align_up(uint64_t v, uint64_t a) {
    return (v + a - 1) & ~(a - 1);
}

void memory_init(void) {
    total_mem = 0;
    free_mem = 0;
    usable_count = 0;
    heap_cursor = 0;
    heap_end = 0;
    ready = 0;

    volatile struct limine_memmap_request req = {
        .id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0
    };
    /* The request must live in the Limine request section. */
    static volatile struct limine_memmap_request memmap_request
        __attribute__((used, section(".requests"))) = {
            .id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0
        };
    (void)req;

    if (!memmap_request.response) return;

    for (uint64_t i = 0; i < memmap_request.response->entry_count; ++i) {
        struct limine_memmap_entry *e = memmap_request.response->entries[i];
        total_mem += e->length;
        if (e->type == LIMINE_MEMMAP_USABLE && usable_count < MAX_RANGES) {
            usable[usable_count].base = e->base;
            usable[usable_count].length = e->length;
            usable_count++;
            free_mem += e->length;
        }
    }

    /* Until paging and a physical-frame allocator exist, keep allocations in a
       kernel-owned static arena. This avoids treating physical addresses as virtual pointers. */
    heap_cursor = (uint64_t)(uintptr_t)kernel_heap;
    heap_end = heap_cursor + sizeof(kernel_heap);
    ready = 1;
}

void *kmalloc(size_t size, size_t alignment) {
    if (!ready || size == 0) return 0;
    if (alignment < 16) alignment = 16;
    if ((alignment & (alignment - 1)) != 0) return 0;
    uint64_t p = align_up(heap_cursor, alignment);
    uint64_t next = align_up(p + size, 16);
    if (next > heap_end) return 0;
    heap_cursor = next;
    if (free_mem >= next - p) free_mem -= next - p;
    return (void *)(uintptr_t)p;
}

uint64_t memory_total(void) { return total_mem; }
uint64_t memory_free(void) { return free_mem; }
int memory_ready(void) { return ready; }
