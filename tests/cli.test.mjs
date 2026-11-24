import test from 'node:test';
import assert from 'node:assert/strict';
import {createServer} from 'node:http';
import {createHash} from 'node:crypto';
import {execFile} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import {decodeFrame} from '../tools/station-cli.mjs';

const cli = fileURLToPath(new URL('../tools/station-cli.mjs', import.meta.url));
const run = (...args) => new Promise(resolve => execFile(process.execPath, [cli, ...args],
  {timeout: 6000}, (err, out, diagnostic) => resolve({code:err?.code ?? 0, out, diagnostic})));
const event = {schema_version:1, type:'event', level:'info', source:'pi', message:'a\nb', ts_ms:1720000000000};
const scan = {schema_version:1, type:'scan_lite', frame_id:'map', points:[[0,1.2]], ts_ms:event.ts_ms};

test('reject malformed samples rather than silently repairing them', () => {
  for (const m of [null, {...event, schema_version:'1'}, {...event, level:'debug'}, {...event, source:''},
    {...event, message:1}, {...event, ts_ms:-1}, {...scan, frame_id:'laser'}, {...scan, points:[[0,1,2]]},
    {...scan, points:[['0',1]]}, {...scan, points:Array(4097).fill([0,0])}]) {
    assert.throws(() => decodeFrame(JSON.stringify(m)));
  }
  assert.throws(() => decodeFrame(new Uint8Array()));
  assert.deepEqual(decodeFrame(JSON.stringify({...scan,points:[]})).points, []);
});

test('print help or reject invalid arguments before connecting', async () => {
  const help = await run('--help');
  assert.equal(help.code, 0);
  assert.match(help.out, /--timeout-ms/);
  for (const args of [[], ['http://localhost/ws'], ['ws://localhost/bad'], ['ws://localhost/ws','--timeout-ms','abc']]) {
    const r = await run(...args);
    assert.equal(r.code, 2);
    assert.equal(r.out, '');
    assert.match(r.diagnostic, /Usage:/);
  }
});

// A loopback fixture only, not a deployable Pi gateway.
async function sample(t, payloads, handshake=true) {
  const server = createServer();
  const sockets = new Set();
  server.on('upgrade', (req, socket) => {
    sockets.add(socket);
    socket.on('error', () => {});
    assert.equal(req.url, '/ws');
    if (!handshake) return socket.destroy();
    const accept = createHash('sha1').update(req.headers['sec-websocket-key']+'258EAFA5-E914-47DA-95CA-C5AB0DC85B11').digest('base64');
    socket.write('HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: '+accept+'\r\n\r\n');
    for (const text of payloads) {
      const b = Buffer.from(text);
      assert.ok(b.length <= 65535);
      const header = b.length < 126 ? Buffer.from([0x81, b.length]) : Buffer.from([0x81, 126, b.length >> 8, b.length & 255]);
      socket.write(Buffer.concat([header, b]));
    }
  });
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  t.after(async () => {
    for (const socket of sockets) socket.destroy();
    await new Promise(resolve => server.close(resolve));
  });
  return run('ws://127.0.0.1:'+server.address().port+'/ws', '--timeout-ms','450');
}

test('native WebSocket emits ordered JSONL and exits at window end', async t => {
  const task = {schema_version:1, type:'task_state', ts_ms:event.ts_ms, seq:42, run_id:'loopback-1',
    state:'running', progress_pct:35, stage_id:'pickup', stages:[{id:'pickup',label:'取料'}]};
  const frames = [event, scan, task];
  const r = await sample(t, frames.map(f => JSON.stringify(f)));
  assert.equal(r.code, 0, r.diagnostic);
  assert.deepEqual(r.out.trimEnd().split('\n').map(JSON.parse), frames);
});
test('empty window is distinguishable from network and protocol errors', async t => {
  assert.equal((await sample(t, [])).code, 4);
  assert.equal((await sample(t, [], false)).code, 1);
  const invalid = await sample(t, ['{}']);
  assert.equal(invalid.code, 3);
  assert.equal(invalid.out, '');
});
