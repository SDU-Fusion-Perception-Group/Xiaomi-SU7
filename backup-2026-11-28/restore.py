import json,hashlib,tarfile,os
from pathlib import Path
b=Path(__file__).resolve().parent
for entry in json.loads((b/'archive-map.json').read_text(encoding='utf-8')):
 if 'original' in entry:
  row=entry['original'];p=b/'home'/row['path'];p.parent.mkdir(parents=True,exist_ok=True)
  with p.open('wb') as f:
   for part in entry['parts']:
    data=(b/part['path']).read_bytes();assert hashlib.sha256(data).hexdigest()==part['sha256'];f.write(data)
  assert hashlib.file_digest(p.open('rb'),'sha256').hexdigest()==row['sha256']
 else:
  p=b/entry['archive'];assert hashlib.file_digest(p.open('rb'),'sha256').hexdigest()==entry['sha256']
  with tarfile.open(p) as t:t.extractall(b/'home',filter='data')
errors=[]
for row in json.loads((b/'source-manifest.json').read_text(encoding='utf-8'))['files']:
 p=b/'home'/row['path']
 if 'symlink' in row:
  if not p.is_symlink():
   if p.exists():p.unlink()
   p.symlink_to(row['symlink'])
  continue
 if not p.is_file() or hashlib.file_digest(p.open('rb'),'sha256').hexdigest()!=row['sha256']:errors.append(row['path'])
 elif os.name=='posix':p.chmod(int(row['mode'],8))
assert not errors,errors
print('All original project files verified. Copy home/* to your Linux home after review.')
