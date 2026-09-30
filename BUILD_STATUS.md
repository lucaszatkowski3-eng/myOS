# Build verification

The bootable ISO is built and smoke-tested with QEMU in GitHub Actions. The USB/HDD image uses a GPT BIOS-boot partition, a FAT boot partition and a persistent MYOSDATA partition.

The current release artifact must only be published after the complete Actions build and QEMU smoke test succeed.
