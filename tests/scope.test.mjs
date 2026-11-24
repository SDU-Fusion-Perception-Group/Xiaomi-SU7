import { beforeEach, test } from 'node:test'
import assert from 'node:assert/strict'
import { createPinia, setActivePinia } from 'pinia'
import { channels, LIMIT, useScope } from '../src/stores/scope.ts'
import { useLink } from '../src/stores/link.ts'
import { decodeFrame } from '../tools/station-cli.mjs'

beforeEach(() => setActivePinia(createPinia()))
const frame = (channel, ts_ms = Date.now()) => ({ schema_version: 1, type: 'telemetry', channel, unit: channels[channel].unit, value: -1.25, ts_ms })
test('tabs select once, remove neighbors, and allow empty selection', () => {
  const s = useScope()
  s.open('motor'); s.open('motor'); s.open('fused'); s.open('gyro'); s.remove('gyro')
  assert.deepEqual(s.tabs, ['motor', 'fused']); assert.equal(s.active, 'motor')
  s.remove('motor'); s.remove('fused'); assert.equal(s.active, null)
})
test('CLI and store accept the same typed channels and reject bad measurements', () => {
  const s = useScope()
  for (const ch of Object.keys(channels)) {
    const f = frame(ch); assert.deepEqual(decodeFrame(JSON.stringify(f)), f); s.ingest(f)
    assert.equal(s.history[ch][0].value, f.value)
    for (const patch of [{ts_ms:-1}, {ts_ms:'100'}, {unit:'?'}, {channel:'constructor'}, {value:'0'}, {value:null}]) {
      const bad = {...f, ...patch}; assert.throws(() => s.ingest(bad)); assert.throws(() => decodeFrame(JSON.stringify(bad)))
    }
  }
  for (const delta of [-1501, 1501]) assert.throws(() => s.ingest(frame('gyro', 5000+delta), 5000))
})
test('history excludes duplicates and reordering; bounds time, memory and display window', () => {
  const s = useScope(), add = ts => s.ingest(frame('gyro', ts), ts)
  add(10); add(10); add(9); assert.equal(s.history.gyro.length, 1)
  for (let i=11;i<=LIMIT+12;i++) add(i)
  assert.equal(s.history.gyro.length,LIMIT); add(40000); assert.equal(s.history.gyro.length,1)
  assert.equal(s.visible('gyro',50000).length,1); assert.equal(s.visible('gyro',50001).length,0)
  s.reset(); assert.deepEqual(s.history.gyro,[])
})
test('link routes actual message payload, rejects bad frame and isolates new session', t => {
  let peer
  const Native = globalThis.WebSocket
  globalThis.window = globalThis
  globalThis.WebSocket = class { constructor(){ peer=this } close(){} }
  t.after(() => { delete globalThis.window; globalThis.WebSocket = Native })
  const link=useLink(), s=useScope(); const connect=()=>{link.connect('ws://localhost/ws');peer.onopen()}
  connect(); const first=peer, f=frame('gyro'); first.onmessage({data:JSON.stringify(f)})
  assert.equal(link.state,'只读在线'); assert.equal(s.history.gyro[0].ts_ms,f.ts_ms)
  connect(); first.onmessage({data:JSON.stringify(f)}); assert.equal(s.history.gyro.length,0)
  peer.onmessage({data:JSON.stringify({...f,unit:'rpm'})}); assert.equal(link.state,'未连接'); assert.equal(s.history.gyro.length,0)
})
