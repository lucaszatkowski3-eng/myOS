#pragma once
#include <stdint.h>
typedef struct { uint8_t bytes[6]; } bt_addr_t;
typedef struct { bt_addr_t address; uint8_t type; char name[64]; } bt_device_t;
int bluetooth_init(void); int bluetooth_scan(bt_device_t *out,uint32_t max); int bluetooth_pair(const bt_addr_t *addr);
