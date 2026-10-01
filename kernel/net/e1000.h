#pragma once
#include <stdint.h>
#define E1000_TX_COUNT 8
#define E1000_RX_COUNT 8
#define E1000_BUF_SIZE 2048
struct e1000_desc { uint64_t addr; uint16_t length; uint8_t cso; uint8_t cmd; uint8_t status; uint8_t errors; uint16_t special; } __attribute__((packed));
struct e1000_device {
 uint64_t mmio_base; uint8_t mac[6]; uint8_t present; uint8_t initialized;
 struct e1000_desc *tx; struct e1000_desc *rx; uint8_t tx_buf[E1000_TX_COUNT][E1000_BUF_SIZE]; uint8_t rx_buf[E1000_RX_COUNT][E1000_BUF_SIZE];
 uint8_t tx_next, rx_next;
};
int e1000_probe(struct e1000_device *dev);
int e1000_init(struct e1000_device *dev);
int e1000_send(struct e1000_device *dev,const void *data,uint16_t length);
int e1000_receive(struct e1000_device *dev,void *data,uint16_t capacity);
