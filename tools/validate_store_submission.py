#!/usr/bin/env python3
import json, re, sys
from pathlib import Path
ROOT=Path("store/submissions")
ID=re.compile(r"^[a-z0-9]+(\\.[a-z0-9-]+)+$")
VER=re.compile(r"^\\d+\\.\\d+\\.\\d+$")
SHA=re.compile(r"^[0-9a-fA-F]{64}$")
errors=[]
for path in sorted(ROOT.glob("*.json")) if ROOT.exists() else []:
    try: data=json.loads(path.read_text())
    except Exception as e: errors.append(f"{path}: invalid JSON: {e}"); continue
    for key in ("id","name","version","entry","license","download","sha256","price"):
        if key not in data: errors.append(f"{path}: missing {key}")
    if "id" in data and not ID.fullmatch(str(data["id"])): errors.append(f"{path}: invalid id")
    if "version" in data and not VER.fullmatch(str(data["version"])): errors.append(f"{path}: invalid version")
    if "sha256" in data and not SHA.fullmatch(str(data["sha256"])): errors.append(f"{path}: invalid sha256")
    if data.get("price") != 0: errors.append(f"{path}: community packages must be free (price=0)")
    if "download" in data and not str(data["download"]).startswith("https://"): errors.append(f"{path}: download must use HTTPS")
    if "permissions" in data and not isinstance(data["permissions"],list): errors.append(f"{path}: permissions must be an array")
if errors:
    print("\n".join(errors)); sys.exit(1)
print("myStore submission metadata: OK")
