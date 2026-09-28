# myStore service

The package manager will maintain an installed-package database, resolve dependencies, verify SHA-256 hashes/signatures and perform atomic install/update/remove transactions.

Network flow:

1. Download signed catalog over HTTPS.
2. Verify catalog signature.
3. Resolve dependencies.
4. Download package.
5. Verify package hash/signature.
6. Install to a versioned directory.
7. Update the installed database.
8. Create desktop/start-menu entries.
