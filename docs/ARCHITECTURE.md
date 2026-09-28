# myOS architecture

myOS is split into five layers:

1. Boot: Limine loads the x86_64 kernel.
2. Kernel: memory, interrupts, scheduler, drivers and VFS.
3. Services: networking, package manager, audio and system services.
4. Desktop: compositor, windows, taskbar, settings and notifications.
5. Applications: isolated user-mode programs installed as packages.

## Application model

Applications are native myOS programs or ports of software whose licenses permit redistribution.

Each application has a unique package ID, semantic version, executable, icon, permissions and dependencies.

## myStore

The store catalog is a verified index of package metadata. A network client will download the catalog over HTTPS, verify package integrity, resolve dependencies and install packages into the system package database.

Commands:

    mystore search <name>
    mystore install <package>
    mystore remove <package>
    mystore update
    mystore list

## Security

The long-term target is user-mode isolation, per-app permissions, package hashes/signatures and a read-only system partition. Network-installed software must never execute merely because a catalog entry exists.
