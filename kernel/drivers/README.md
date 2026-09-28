# Driver layer

myOS separates hardware drivers from the kernel core. The driver API covers storage, PCI/network, USB/HID, audio, GPU and Bluetooth.

Real hardware support is added per controller/chipset. The first supported test target is QEMU x86_64; physical WLAN/Bluetooth adapters are added after the PCI/USB and interrupt layers are operational.
