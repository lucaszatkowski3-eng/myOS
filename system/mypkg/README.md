# myPkg package manager

myPkg is the package-management service for myOS.

Current responsibilities:
- package IDs and semantic versions
- installed-package database
- dependency metadata
- package state transitions: available -> installed -> removed
- verification hooks for SHA-256 and signatures

The current kernel milestone keeps the database in memory. Persistent VFS storage and HTTPS downloads are separate milestones and must be implemented before claiming full online installation.
