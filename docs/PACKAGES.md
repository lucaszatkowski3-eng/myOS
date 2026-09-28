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
