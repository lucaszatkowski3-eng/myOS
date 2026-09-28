# myOS

myOS is a 64-bit hobby operating system project for x86_64 PCs.

## Current milestone

- Limine-based x86_64 boot
- UEFI/BIOS boot image generation
- linear framebuffer graphics
- desktop/window UI
- keyboard input
- PS/2 mouse input
- Start menu
- Terminal
- Calculator
- Notepad
- File Manager (in-memory demo filesystem)
- GitHub Actions build

This is an original operating-system implementation. It does not contain Windows code or proprietary Windows components.

## Build

Linux/WSL:

```bash
make
```

The result is `build/myos.iso`.

Test with QEMU:

```bash
make run
```

The CI workflow builds the ISO automatically.

## Roadmap

1. persistent filesystem and block-device driver
2. process/user-mode architecture
3. real executable loader
4. ACPI, PCI and USB
5. networking
6. audio
7. package manager and SDK
8. richer desktop applications
9. security model and user accounts
