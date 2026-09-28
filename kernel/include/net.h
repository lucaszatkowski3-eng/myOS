#pragma once
#include <stdint.h>
typedef struct { uint8_t bytes[6]; } mac_addr_t;
typedef struct net_device { char name[16]; mac_addr_t mac; uint32_t mtu; int (*up)(struct net_device*); int (*send)(struct net_device*,const void*,uint32_t); } net_device_t;
int net_init(void); int net_register(net_device_t *dev); int net_dhcp_start(net_device_t *dev); int net_https_get(const char *host,const char *path);
