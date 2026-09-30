# Community publishing for myStore

myStore is designed as a free community software catalog. Publishing has no listing fee and packages are intended to be distributed through the author's own public GitHub repository/release.

## Package rules

Every submission must contain a valid .mypkg package and manifest. The package must declare its ID, version, entry executable, license and requested permissions. The package validator checks the archive structure, manifest, executable format and SHA-256 before a package is accepted into the catalog.

Native apps should use the myOS executable format. The current kernel also contains the first myX executable runtime, which provides a small sandboxable graphics/input ABI. Full user-mode isolation and ELF execution are still separate kernel milestones.

## Free publishing flow

1. Developer creates a package with tools/mkpackage.py.
2. Developer publishes the package as a GitHub Release asset.
3. Developer submits catalog metadata or a pull request to the myOS store index.
4. Automated validation checks the package.
5. Once accepted, myStore can list the package with price 0.
6. Users install it into their persistent VFS.

GitHub Releases support distributing binary release assets and stable download URLs, which makes them suitable as the initial free hosting mechanism for community packages.

## Permissions

Packages must request only what they need. Examples:

- filesystem.user
- graphics
- keyboard
- network
- process.spawn

Community packages must not receive kernel privileges. The long-term runtime will execute third-party applications in user mode with a permission boundary.

## Moderation and removal

Free publishing does not mean unrestricted kernel access. Invalid, malicious or infringing packages can be rejected or removed from the catalog. The package itself remains under its author's license.

## Current limitation

The repository contains the catalog and publishing specification, but a public submission web service and a complete user-mode sandbox are not finished yet. Until those are implemented, the practical submission path is GitHub Release + catalog contribution.
