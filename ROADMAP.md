# myOS completion roadmap

The target is a usable desktop OS, not a visual mock-up.

## Kernel
- [x] Limine x86_64 boot
- [x] framebuffer output
- [ ] GDT/IDT and exception handling
- [ ] physical/virtual memory manager
- [ ] preemptive scheduler
- [ ] system calls
- [ ] user mode
- [ ] ELF loader

## Hardware
- [ ] PCI
- [ ] ACPI
- [ ] AHCI/NVMe storage
- [ ] USB HID
- [ ] USB mass storage
- [ ] Intel/Realtek network drivers
- [ ] audio
- [ ] display acceleration

## Storage
- [ ] VFS
- [ ] persistent filesystem
- [ ] partitions
- [ ] permissions
- [ ] per-user home directories

## Desktop
- [ ] compositor
- [ ] real mouse cursor
- [ ] keyboard driver
- [ ] movable/resizable windows
- [ ] taskbar
- [ ] start menu
- [ ] notifications
- [ ] settings
- [ ] login/user accounts

## Applications
- [ ] application ABI
- [ ] terminal
- [ ] file manager
- [ ] text editor
- [ ] calculator
- [ ] image viewer
- [ ] media player
- [ ] browser integration
- [ ] system monitor

## myStore
- [x] catalog schema
- [x] package manifest format
- [x] package builder
- [ ] package database
- [ ] dependency resolver
- [ ] SHA-256 verification in OS
- [ ] package signatures
- [ ] HTTPS client
- [ ] store GUI
- [ ] install/remove/update transactions
- [ ] rollback

## Games
First-party games use the same package system as applications. Third-party/open-source games can be ported when their licenses permit distribution.

The store must never execute arbitrary downloaded content in kernel mode.
