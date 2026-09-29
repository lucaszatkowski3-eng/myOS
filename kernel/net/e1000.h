#pragma once
#include <stdint.h>

/* QEMU/Intel e1000-compatible NIC support target. */
struct e1000_device {
    uint64_t mmio_base;
    uint8_t mac[6];
    uint8_t present;
};

int e1000_probe(struct e1000_device *dev);
int e1000_init(struct e1000_device *dev);
int e1000_send(struct e1000_device *dev, const void *data, uint16_t length);
