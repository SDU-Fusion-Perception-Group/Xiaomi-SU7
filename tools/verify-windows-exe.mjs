import { createHash } from 'node:crypto';
import { readFileSync } from 'node:fs';
import { pathToFileURL } from 'node:url';

export function verifyWindowsExe(bytes) {
  if (bytes.length < 64 || bytes.readUInt16LE(0) !== 0x5a4d) throw new Error('Missing MZ header');
  const pe = bytes.readUInt32LE(0x3c);
  if (pe < 64 || pe + 24 > bytes.length || bytes.readUInt32LE(pe) !== 0x4550) throw new Error('Invalid PE header');
  const flags = bytes.readUInt16LE(pe + 22);
  if (bytes.readUInt16LE(pe + 4) !== 0x8664 || !(flags & 2) || (flags & 0x2000)) throw new Error('Expected an AMD64 executable, not a DLL');
  const optionalSize = bytes.readUInt16LE(pe + 20);
  if (optionalSize < 112 || pe + 24 + optionalSize > bytes.length || bytes.readUInt16LE(pe + 24) !== 0x20b) throw new Error('Invalid PE32+ optional header');
  if (bytes.readUInt16LE(pe + 24 + 68) !== 2) throw new Error('Expected Windows GUI subsystem');
  return { architecture: 'x64', bytes: bytes.length, sha256: createHash('sha256').update(bytes).digest('hex') };
}

if (process.argv[1] && pathToFileURL(process.argv[1]).href === import.meta.url) {
  try { console.log(JSON.stringify(verifyWindowsExe(readFileSync(process.argv[2])))); }
  catch (error) { console.error(error.message); process.exitCode = 1; }
}
