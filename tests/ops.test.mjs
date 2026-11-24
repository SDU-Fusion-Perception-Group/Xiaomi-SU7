import { beforeEach, test } from 'node:test'
import assert from 'node:assert/strict'
import { createPinia, setActivePinia } from 'pinia'
import { filterLogs, LOG_LIMIT, useOps } from '../src/stores/ops.ts'

beforeEach(() => setActivePinia(createPinia()))

test('retains only the latest records, including for error views', () => {
  const s = useOps()
  s.add('error', 'mock', 'expired')
  for (let i = 0; i < LOG_LIMIT; i++) s.add('info', 'mock', String(i))
  assert.equal(s.entries.length, LOG_LIMIT)
  assert.equal(s.entries[0].id, 2)
  assert.equal(s.entries.at(-1).id, LOG_LIMIT + 1)
  assert.equal(s.errors.length, 0)
})

test('filters level together with case-insensitive source or message', () => {
  const s = useOps()
  s.add('info', 'Radar', 'ready')
  s.add('warn', 'sim', 'RADAR slow')
  s.add('error', 'map', 'timeout')
  assert.deepEqual(filterLogs(s.entries, 'all', ' radar ').map(x => x.id), [1, 2])
  assert.deepEqual(filterLogs(s.entries, 'warn', ' radar ').map(x => x.id), [2])
  assert.deepEqual(s.errors.map(x => x.id), [3])
})

test('JSONL round-trips structured UTC records including quotes and newlines', () => {
  const s = useOps()
  assert.equal(s.jsonl(), '')
  s.add('error', '模拟', '引号 "\n第二行')
  s.add('warn', 'sensor', 'delay')
  const records = s.jsonl().trimEnd().split('\n').map(line => JSON.parse(line))
  assert.deepEqual(records, s.entries)
  assert.equal(new Date(records[0].timestamp).toISOString(), records[0].timestamp)
})

test('diagnostics only recognize complete read-only commands', () => {
  const s = useOps()
  const run = cmd => s.diagnose(cmd, 'ready', null)
  assert.match(run('help'), /status.*errors.*logs/)
  assert.match(run(' STATUS '), /ready.*等待数据/)
  assert.match(s.diagnose('status', 'running', 0), /running.*候选目标：0/)
  assert.match(run('errors'), /暂无错误/)
  s.add('error', 'mock', 'fault')
  s.add('info', 'sensor', 'online')
  assert.match(run('logs'), /online[\s\S]*fault/)
  assert.doesNotMatch(run('errors'), /online/)
  assert.match(run('errors'), /fault/)
  for (const cmd of ['status; logs', 'logs | more', 'cmd', '']) assert.match(run(cmd), /未知命令/)
  assert.equal(s.entries.length, 2)
})
