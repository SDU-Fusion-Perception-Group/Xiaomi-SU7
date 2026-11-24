#!/usr/bin/env node
import { parseArgs } from 'node:util';
import { pathToFileURL } from 'node:url';
import { validateTaskState } from '../src/task-protocol.mjs';

const help = [
  'Usage: node tools/station-cli.mjs ws://HOST:PORT/ws [--timeout-ms N]',
  'Read-only WebSocket sampler. Prints one validated JSON object per line.',
  'Total window including handshake: 10000 ms by default. -h, --help: show help.',
  'Exit: 0 sampled/help; 1 transport/runtime/output; 2 arguments; 3 protocol; 4 empty window.',
].join('\n');

export function decodeFrame(text) {
  if (typeof text !== 'string') throw Error('expected a text message');
  const m = JSON.parse(text);
  if (m?.schema_version !== 1) throw Error('expected schema_version:1');
  if (!Number.isSafeInteger(m.ts_ms) || m.ts_ms < 0) throw Error('invalid ts_ms');
  if (m.type === 'event') {
    if (!['info', 'warn', 'error'].includes(m.level)) throw Error('invalid event level');
    for (const field of ['source', 'message']) {
      if (typeof m[field] !== 'string' || !m[field].trim()) throw Error('invalid event ' + field);
    }
  } else if (m.type === 'task_state') {
    validateTaskState(m);
  } else if (m.type === 'telemetry') {
    const units = new Map([['gyro', '°/s'], ['motor', 'rpm'], ['fused', '°'], ['fused_x', 'm'], ['fused_y', 'm']]);
    if (!units.has(m.channel) || units.get(m.channel) !== m.unit || !Number.isFinite(m.value)) {
      throw Error('invalid telemetry channel, unit or value');
    }
  } else if (m.type === 'scan_lite') {
    if (m.frame_id !== 'map') throw Error('expected frame_id:map');
    if (!Array.isArray(m.points) || m.points.length > 4096 || !m.points.every(
      p => Array.isArray(p) && p.length === 2 && p.every(Number.isFinite),
    )) throw Error('expected at most 4096 finite [x,y] pairs');
  } else throw Error('unsupported message type');
  return m;
}

function main(args) {
  let url, timeout;
  try {
    const {positionals, values} = parseArgs({args, allowPositionals: true, options: {
      help: {type: 'boolean', short: 'h'}, 'timeout-ms': {type: 'string', default: '10000'},
    }});
    if (values.help) return console.log(help);
    if (positionals.length !== 1) throw Error('supply one endpoint');
    url = new URL(positionals[0]);
    if (!['ws:', 'wss:'].includes(url.protocol) || url.pathname !== '/ws' || url.hash || url.username || url.password) throw Error('expected ws(s)://HOST:PORT/ws');
    timeout = Number(values['timeout-ms']);
    if (!/^\d+$/.test(values['timeout-ms']) || timeout < 1 || timeout > 2147483647) throw Error('invalid --timeout-ms');
  } catch (e) {
    console.error(e.message + '\n' + help);
    process.exitCode = 2;
    return;
  }
  if (typeof WebSocket !== 'function') throw Error('Node with native WebSocket required (use Node 22+).');
  const ws = new WebSocket(url);
  let sampled = false, finished = false;
  const timer = setTimeout(() => finish(sampled ? 0 : 4, sampled ? '' : 'timeout: no frames'), timeout);
  function finish(code, reason) {
    if (finished) return;
    finished = true;
    clearTimeout(timer);
    if (reason) console.error(reason);
    ws.close();
    // Flush accepted lines, but never wait for a stalled peer to acknowledge close.
    process.stdout.write('', () => process.exit(code));
  }
  process.stdout.on('error', () => process.exit(1));
  ws.addEventListener('message', ({data}) => {
    if (finished) return;
    let m;
    try { m = decodeFrame(data); }
    catch (e) { return finish(3, 'protocol: ' + e.message); }
    sampled = true;
    process.stdout.write(JSON.stringify(m) + '\n');
  });
  ws.addEventListener('error', () => finish(1, 'WebSocket connection failed'));
  ws.addEventListener('close', e => {
    const ok = sampled && (e.code === 1000 || e.code === 1005);
    finish(ok ? 0 : 1, ok ? '' : 'WebSocket closed: ' + e.code);
  });
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try { main(process.argv.slice(2)); }
  catch (e) { console.error(e.message); process.exitCode = 1; }
}
