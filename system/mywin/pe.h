#pragma once
#include <stdint.h>
struct pe_info { uint8_t valid; uint8_t is_64bit; uint16_t machine; uint32_t sections; };
int pe_inspect(const void *image, uint32_t size, struct pe_info *out);
