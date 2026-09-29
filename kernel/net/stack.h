#pragma once
#include <stdint.h>

struct net_ipv4 {
    uint8_t octet[4];
};

struct net_config {
    struct net_ipv4 address;
    struct net_ipv4 gateway;
    struct net_ipv4 dns;
    uint8_t dhcp_ready;
};

void net_stack_init(void);
int net_dhcp_start(struct net_config *config);
int net_dns_lookup(const char *name, struct net_ipv4 *result);
