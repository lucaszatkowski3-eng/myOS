# myStore

myStore is the graphical package catalog and installer frontend for myOS.

First-party applications:
- myWrite — documents
- myCalc — spreadsheets
- mySlides — presentations
- Terminal
- File Manager
- Calculator

Installation is delegated to myPkg. A catalog entry is metadata, not executable permission: a package must pass manifest/hash/signature checks and an explicit install transaction before it can be run.

The current early-kernel milestone keeps package state in memory. Persistent VFS storage and HTTPS downloads are separate milestones.
