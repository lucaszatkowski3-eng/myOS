#include "e1000.h"

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0,%1" :: "a"(value), "Nd"(port));
}
static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1,%0" : "=a"(value) : "Nd"(port));
    return value;
}

/*
 * PCI config-space access for the first network milestone.
 * The driver deliberately only targets Intel e1000-compatible hardware.
 */
static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t off) {
    uint32_t address = 0x80000000u |
        ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
        ((uint32_t)func << 8) | (off & 0xfcu);
    outl(0xCF8, address);
    return inl(0xCFC);
}

int e1000_probe(struct e1000_device *dev) {
    if (!dev) return -1;
    dev->present = 0;

    for (uint16_t bus = 0; bus < 256; ++bus) {
        for (uint8_t slot = 0; slot < 32; ++slot) {
            for (uint8_t func = 0; func < 8; ++func) {
                uint32_t id = pci_read32((uint8_t)bus, slot, func, 0);
                uint16_t vendor = (uint16_t)(id & 0xffff);
                uint16_t device = (uint16_t)(id >> 16);

                /* Intel vendor + common e1000/QEMU device IDs. */
                if (vendor == 0x8086 &&
                    (device == 0x100e || device == 0x100f ||
                     device == 0x10d3 || device == 0x153a)) {
                    uint32_t bar0 = pci_read32((uint8_t)bus, slot, func, 0x10);
                    if (!(bar0 & 1u)) {
                        dev->mmio_base = (uint64_t)(bar0 & ~0xFu);
                        dev->present = 1;
                        return 0;
                    }
                }
            }
        }
    }
    return -1;
}

int e1000_init(struct e1000_device *dev) {
    if (!dev || !dev->present) return -1;
    /*
     * RX/TX descriptor rings and interrupt setup are the next driver step.
     * Keeping this interface small lets the ARP/IP stack remain hardware-neutral.
     */
    return 0;
}

int e1000_send(struct e1000_device *dev, const void *data, uint16_t length) {
    (void)dev; (void)data; (void)length;
    return -1; /* TX ring is intentionally not exposed until it is initialized. */
}
