# myWin compatibility layer

myWin is the Windows PE compatibility service.

It will identify PE32/PE32+ executables, map supported Windows paths to myOS VFS and provide a Win32 compatibility surface. Native myOS programs use the ELF ABI.

The first milestone is safe PE detection and a launcher boundary. Full arbitrary .exe compatibility is a large project because Windows applications depend on many Windows APIs and runtime libraries; it is not something that can honestly be enabled by a small kernel patch.
