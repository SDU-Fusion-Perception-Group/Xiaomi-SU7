import assert from 'node:assert/strict'
import { test } from 'node:test'
import { createServer } from 'vite'
import { createSSRApp, h } from 'vue'
import { renderToString } from 'vue/server-renderer'

test('Pi task panel renders progress, current stage and stale status without controls', async () => {
  const vite = await createServer({ server: { middlewareMode: true, watch: null, ws: false }, appType: 'custom' })
  try {
    const { default: Panel } = await vite.ssrLoadModule('/src/components/PiTaskStatus.vue')
    const task = { state: 'estop', progress_pct: 35, run_id: 'run-1', stage_id: 'pickup',
      stages: [{ id:'pickup',label:'取料' }, { id:'deliver',label:'放料' }], detail: '底盘报告急停' }
    const render = props => renderToString(createSSRApp({ render: () => h(Panel, props) }))
    const live = await render({ task, stale: false })
    assert.match(live, /急停锁定/)
    assert.match(live, /<progress value="35" max="100"/)
    assert.match(live, /aria-current="step"[^>]*>取料/)
    assert.doesNotMatch(live, /<button/)
    assert.match(await render({ task, stale: true }), /状态已过期/)
    assert.match(await render({ task: null, stale: false }), /等待任务快照/)
    const { default: Monitor } = await vite.ssrLoadModule('/src/components/PiTaskMonitor.vue')
    const tasks = new Map([
      ['run-1', { snapshot: task, stale: false }],
      ['run-2', { snapshot: { ...task, run_id:'run-2', state:'paused', progress_pct:75, detail:'第二任务详情' }, stale: true }],
    ])
    const monitor = (...ids) => renderToString(createSSRApp({ render: () => h(Monitor, { tasks, hiddenRuns: new Set(ids) }) }))
    const both = await monitor()
    assert.match(both, /<progress value="35"/)
    assert.match(both, /<progress value="75"/)
    assert.match(both, /状态已过期 · 已暂停/)
    const filtered = await monitor('run-2')
    assert.match(filtered, /<input type="checkbox"[^>]*><span[^>]*>run-2/)
    assert.match(filtered, /<progress value="35"/)
    assert.doesNotMatch(filtered, /<progress value="75"|第二任务详情/)
    assert.match(await monitor('run-1','run-2'), /未选中任务/)
  } finally { await vite.close() }
})
