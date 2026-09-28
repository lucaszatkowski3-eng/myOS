# Network stack

Target stack:

NIC driver -> Ethernet -> ARP/IPv6 ND -> IP -> UDP/TCP -> DNS -> TLS -> HTTP(S).

Wi-Fi is intentionally separated from IP so different adapters can share the same network stack. Bluetooth uses a separate HCI/L2CAP/GATT path.
