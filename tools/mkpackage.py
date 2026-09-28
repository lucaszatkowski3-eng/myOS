#!/usr/bin/env python3
import hashlib, json, pathlib, sys, zipfile

if len(sys.argv) != 3:
    print('usage: mkpackage.py <package-directory> <output.mypkg>')
    raise SystemExit(2)
root = pathlib.Path(sys.argv[1]).resolve()
out = pathlib.Path(sys.argv[2]).resolve()
manifest_path = root / 'manifest.json'
manifest = json.loads(manifest_path.read_text())
for key in ('id','name','version','entry'):
    if key not in manifest:
        raise SystemExit(f'missing manifest field: {key}')
entry = root / manifest['entry']
if not entry.is_file():
    raise SystemExit(f'entry does not exist: {entry}')
with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
    for p in root.rglob('*'):
        if p.is_file():
            z.write(p, p.relative_to(root).as_posix())
sha = hashlib.sha256(out.read_bytes()).hexdigest()
print(json.dumps({'id':manifest['id'],'version':manifest['version'],'file':str(out),'sha256':sha}, indent=2))
