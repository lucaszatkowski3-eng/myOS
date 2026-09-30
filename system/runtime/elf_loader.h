#pragma once
#include <stdint.h>
struct elf64_info { uint8_t valid; uint16_t machine; uint16_t type; uint64_t entry; uint16_t phnum; };
int elf64_inspect(const void *image, uint64_t size, struct elf64_info *out);
