# myOS package format

The .mypkg format is a ZIP-compatible archive containing:

    manifest.json
    bin/<executable>
    assets/icon.svg

Example manifest:

    {
      "id": "org.example.app",
      "name": "Example App",
      "version": "1.0.0",
      "entry": "bin/example",
      "license": "MIT",
      "permissions": [],
      "dependencies": []
    }

The package manager must verify package ID/version, manifest syntax, executable path, dependencies, SHA-256 hash and signatures when enabled.

Install destination: /system/apps/<package-id>/<version>/
Per-user data: /home/<user>/.local/share/<package-id>/


## Executable targets

A package entry can use a native myOS executable or the myX runtime format.

A myX executable starts with the MYX1 header and contains bounded drawing/input bytecode. The runtime validates the header and instruction boundaries before execution. This is intended as a small first executable target while the full user-mode ELF/process subsystem is developed.

Community packages must never receive kernel privileges. Requested permissions are metadata for the future user-mode permission layer.
