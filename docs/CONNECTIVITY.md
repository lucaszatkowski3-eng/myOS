# myOS connectivity

## Goal

myOS is designed to make moving files between a phone and a PC simple.

The desktop already contains **myShare**, a dedicated transfer workspace, and **myBrowser**, the native browser application shell.

## Current state

- Persistent files can be stored by myVFS on supported ATA disks.
- myShare can create files in the persistent VFS and exposes the transfer workflow in the desktop.
- myBrowser has an address bar and native UI.
- The current network driver only detects supported Intel e1000 hardware. It does not yet provide Ethernet TX/RX, DHCP, DNS, TCP or HTTP.
- USB device and USB mass-storage support are not implemented yet.

Therefore myOS does **not** currently pretend that Internet browsing or phone transfer is already working over the network.

## Next connectivity layer

1. Complete e1000 RX/TX rings.
2. Add Ethernet frames and ARP.
3. Add IPv4 + ICMP.
4. Add UDP, DHCP and DNS.
5. Add TCP.
6. Add a small HTTP client for myBrowser.
7. Add a small HTTP transfer service/client for myShare.
8. Add USB host + mass-storage support as a second transfer path.

The intended user flow is:

**Phone → myShare → choose file → transfer → /home → File Manager**

and for browsing:

**myBrowser → URL → HTTP/HTTPS networking → rendered page**

HTTPS should be added only after a suitable TLS implementation is available; a browser must not silently claim secure connections without TLS.
