"""Create a reviewable v0.6.2 source package with unchanged documentation references."""
from pathlib import Path
import hashlib
import json
import zipfile

root=Path(__file__).resolve().parents[1]
assert 'version: \"0.6.2\"' in (root/'lwz303-sol-example.yaml').read_text(encoding='utf-8')
assert 'version: "0.6.2"' in (root/'lwz303.yaml').read_text(encoding='utf-8')
assert 'Starting LWZ component v0.6.2' in (root/'components/lwz2/lwz2.cpp').read_text(encoding='utf-8')
assert 'v0.6.2' in (root/'README.md').read_text(encoding='utf-8').splitlines()[0]
out=root/'dist/lwz303-v0.6.2'
out.mkdir(parents=True,exist_ok=True)
source=root/'lwz303-sol-example.yaml'
local=source.read_text(encoding='utf-8').replace('source: github://vandecook/lwz303-to-esp32','source: components',1)
(out/'lwz303-sol-local.yaml').write_text(local,encoding='utf-8')
user_local=(root/'lwz303.yaml').read_text(encoding='utf-8').replace('source: github://vandecook/lwz303-to-esp32','source: components',1)
(out/'lwz303-local.yaml').write_text(user_local,encoding='utf-8')
files=[root/name for name in ('README.md','CHANGELOG.md','ROADMAP.md','VALIDATION.md','LICENSE',
                              'lwz303.yaml','lwz303-sol-example.yaml','.gitignore','.github/workflows/validate.yaml')]
files+=sorted(p for p in (root/'components/lwz2').iterdir() if p.suffix in {'.cpp','.h','.py','.json','.js','.txt'})
files+=sorted((root/'docs').glob('*.md'))
files+=sorted(p for p in (root/'tests').iterdir() if p.suffix in {'.py','.cpp','.h'})
files+=sorted((root/'ha').glob('*.yaml'))
files.append(root/'research/package_v062.py')
files.append(root/'research/build_web_help.py')
files+=sorted(p for p in (root/'web').iterdir() if p.suffix in {'.js','.json','.txt'})
refs=root/'docs/references'
manifest=json.loads((refs/'SOURCES.json').read_text(encoding='utf-8'))
files+=[refs/'README.md',refs/'SOURCES.json']
for entry in manifest['references']:
    p=refs/entry['file']
    assert hashlib.sha256(p.read_bytes()).hexdigest()==entry['sha256'], p
    files.append(p)
archive=out/'lwz303-v0.6.2.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
    for p in files:z.write(p,p.relative_to(root).as_posix())
    z.write(out/'lwz303-sol-local.yaml','lwz303-sol-local.yaml')
    z.write(out/'lwz303-local.yaml','lwz303-local.yaml')
    hashes={p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
    hashes['lwz303-sol-local.yaml']=hashlib.sha256((out/'lwz303-sol-local.yaml').read_bytes()).hexdigest()
    hashes['lwz303-local.yaml']=hashlib.sha256((out/'lwz303-local.yaml').read_bytes()).hexdigest()
    z.writestr('PACKAGE-SHA256.json',json.dumps(hashes,indent=2)+'\n')
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
    for name,digest in json.loads(z.read('PACKAGE-SHA256.json')).items():
        assert hashlib.sha256(z.read(name)).hexdigest()==digest,name
    assert len(z.namelist())==len(set(z.namelist()))
    assert not any(n.endswith('secrets.yaml') or '__pycache__' in n or n.endswith('.bin') for n in z.namelist())
    assert len([n for n in z.namelist() if n.endswith('.pdf')])==2
    assert 'source: components' in z.read('lwz303-sol-local.yaml').decode('utf-8')
    assert 'source: components' in z.read('lwz303-local.yaml').decode('utf-8')
    assert z.read('components/lwz2/web_help.js') == (root/'web/lwz-webserver-v3-with-help.js').read_bytes()
    for config_name in ('lwz303.yaml','lwz303-sol-example.yaml','lwz303-local.yaml','lwz303-sol-local.yaml'):
        config_text = z.read(config_name).decode('utf-8')
        assert 'web_help: true' in config_text
        assert 'js_include:' not in config_text
print(f'v0.6.2 source package: {archive.name}; {len(files)+3} files; {archive.stat().st_size/1024/1024:.2f} MiB; references verified')
digest = hashlib.sha256(archive.read_bytes()).hexdigest()
(out / (archive.name + '.sha256')).write_text(digest + '  ' + archive.name + '\n', encoding='utf-8')
