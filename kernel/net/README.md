# Network stack

Target stack:

NIC driver -> Ethernet -> ARP/IPv4/IPv6 -> UDP/TCP -> DNS -> TLS -> HTTP(S).

## Hardware milestones

The first deterministic target is the emulated Intel e1000 adapter used by QEMU. It gives myOS a repeatable Ethernet device for driver and packet-stack testing.

Wi-Fi is intentionally separate from IP because different laptops expose different wireless controllers and often require different firmware. Once PCI enumeration, USB, and the network stack are stable, chipset-specific Wi-Fi drivers can be added.

Bluetooth uses a separate HCI/L2CAP/GATT path.
