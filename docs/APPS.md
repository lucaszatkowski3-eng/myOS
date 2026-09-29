# myOS applications

## First-party suite

### myWrite
Document editor with text editing and file saving.

### myCalc
Spreadsheet-style application with cells, arithmetic formulas and CSV support.

### mySlides
Presentation editor with slides, text and navigation.

### myStore
Application catalog and installer frontend.

### Terminal
Command-line access to system services.

### File Manager
Browsing and manipulating the VFS.

## Architecture

The long-term target is user-mode ELF applications communicating with the kernel through syscalls. During the early graphical milestones, first-party applications may be built into the system image so the UI can be tested before process isolation and the ELF loader are complete.
