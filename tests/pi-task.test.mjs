import { test } from 'node:test'
import assert from 'node:assert/strict'
import { createPinia, setActivePinia } from 'pinia'
import { useLink } from '../src/stores/link.ts'
import { useOps } from '../src/stores/ops.ts'
import { decodeFrame } from '../tools/station-cli.mjs'

const task = (patch = {}) => ({ schema_version: 1, type: 'task_state', ts_ms: Date.now(), seq: 4,
  run_id: 'run-1', state: 'running', progress_pct: 35, stage_id: 'pickup',
  stages: [{ id: 'pickup', label: '取料' }, { id: 'deliver', label: '放料' }], ...patch })

test('CLI validates Pi task snapshots and state-dependent invariants', () => {
  for (const f of [task(), task({state:'succeeded',stage_id:null,progress_pct:100}), task({state:'idle',run_id:null,stages:[],stage_id:null,progress_pct:0})]) assert.deepEqual(decodeFrame(JSON.stringify(f)), f)
  for (const patch of [{seq:-1},{stage_id:'missing'},{progress_pct:101},{progress_pct:'25'},
    {state:'unknown'},{state:'succeeded'},{run_id:null},{stages:[{id:'pickup',label:'A'},{id:'pickup',label:'B'}]},{detail:' '.repeat(257)}]) {
    assert.throws(() => decodeFrame(JSON.stringify(task(patch))), /task_state/)
  }
})

test('Pi event and task share a socket; out-of-order snapshots and old sessions do not overwrite', t => {
  setActivePinia(createPinia())
  t.mock.timers.enable({ apis: ['setTimeout'] })
  const previous = globalThis.WebSocket
  let peer
  globalThis.window = globalThis
  globalThis.WebSocket = class { constructor() { peer = this } close() {} }
  t.after(() => { delete globalThis.window; globalThis.WebSocket = previous })
  const link = useLink(), ops = useOps()
  const connect = () => { link.connect('ws://localhost/ws'); peer.onopen() }
  const send = (f, socket = peer) => socket.onmessage({data:JSON.stringify(f)})
  connect()
  const first = peer
  send(task())
  const a = () => link.tasks.get('run-1')
  const b = () => link.tasks.get('run-2')
  t.mock.timers.tick(1000)
  send(task({seq:5,run_id:'run-2',progress_pct:70}))
  link.setTaskVisible('run-2', false)
  send(task({seq:6,run_id:'run-2',progress_pct:75}))
  send(task({seq:5,progress_pct:1}))
  assert.equal(a().snapshot.progress_pct, 35)
  assert.equal(b().snapshot.progress_pct, 75)
  assert.equal(link.hiddenRuns.has('run-2'), true)
  t.mock.timers.tick(2500)
  assert.equal(a().stale, true)
  assert.equal(b().stale, false)
  send(task({seq:7,state:'estop',detail:'底盘报告急停'}))
  assert.equal(a().snapshot.state, 'estop')
  assert.equal(a().stale, false)
  link.setTaskVisible('run-2', true)
  assert.equal(link.hiddenRuns.has('run-2'), false)
  send(task({seq:8,state:'idle',run_id:null,stage_id:null,progress_pct:0,stages:[]}))
  assert.equal(link.tasks.size, 2)
  assert.equal(link.tasks.has(null), false)
  link.setTaskVisible('run-2', false)
  send({schema_version:1,type:'event',ts_ms:1000,source:'mission',level:'warn',message:'底盘报告急停'})
  assert.equal(ops.entries.at(-1).timestamp, '1970-01-01T00:00:01.000Z')
  assert.equal(ops.entries.at(-1).source, 'Pi / mission')
  peer.onclose({wasClean:true,code:1000})
  assert.equal(link.tasks.size, 0)
  connect()
  send(task({seq:6}), first)
  assert.equal(link.tasks.size, 0)
  send(task({seq:0,run_id:'run-2'}))
  assert.equal(link.tasks.get('run-2').snapshot.run_id, 'run-2')
  assert.equal(link.hiddenRuns.size, 0)
  send(task({seq:1,progress_pct:200}))
  assert.equal(link.state, '未连接')
  assert.equal(link.tasks.size, 0)
})
