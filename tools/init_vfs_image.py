#!/usr/bin/env python3
import struct, sys
SECTOR=512
LBA=65536
MAGIC=0x31534656
VERSION=1
if len(sys.argv)!=2: raise SystemExit("usage: init_vfs_image.py IMAGE")
with open(sys.argv[1],"r+b") as f:
    f.seek(LBA*SECTOR)
    f.write(struct.pack("<IIII",MAGIC,VERSION,0,0)+bytes(SECTOR-16))
    f.write(bytes(8*SECTOR))
print(f"Initialized myVFS data area at LBA {LBA}")
