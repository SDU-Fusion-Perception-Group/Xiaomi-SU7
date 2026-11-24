import { test } from 'node:test';
import assert from 'node:assert/strict';
import { verifyWindowsExe } from '../tools/verify-windows-exe.mjs';

function guiHeader() {
  const b = Buffer.alloc(256);
  b.writeUInt16LE(0x5a4d, 0); b.writeUInt32LE(64, 60);
  b.writeUInt32LE(0x4550, 64); b.writeUInt16LE(0x8664, 68);
  b.writeUInt16LE(112, 84); b.writeUInt16LE(2, 86);
  b.writeUInt16LE(0x20b, 88); b.writeUInt16LE(2, 156);
  return b;
}
test('release verifier identifies and hashes an x64 GUI header', () => {
  const value = verifyWindowsExe(guiHeader());
  assert.equal(value.architecture, 'x64');
  assert.match(value.sha256, /^[a-f0-9]{64}$/);
});
test('release verifier rejects truncated, wrong architecture and DLL headers', () => {
  assert.throws(() => verifyWindowsExe(guiHeader().subarray(0, 101)), /PE32/);
  const b = guiHeader(); b.writeUInt16LE(0xaa64, 68);
  assert.throws(() => verifyWindowsExe(b), /AMD64/);
  b.writeUInt16LE(0x8664, 68); b.writeUInt16LE(0x2002, 86);
  assert.throws(() => verifyWindowsExe(b), /DLL/);
});
