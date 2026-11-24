<script setup lang="ts">
import type { TaskSnapshot, TaskState } from '../task-protocol.mjs'
defineProps<{ task: TaskSnapshot | null; stale: boolean }>()
const labels: Record<TaskState, string> = {
  idle: '待命', running: '执行中', paused: '已暂停', estop: '急停锁定',
  succeeded: '已完成', failed: '失败', cancelled: '已取消',
}
</script>

<template>
  <section class="pi-task" aria-label="Pi 任务状态" :data-alert="!stale && (task?.state === 'estop' || task?.state === 'failed')">
    <header><h2>Pi 任务</h2><span>只读</span></header>
    <p v-if="!task" role="status">等待任务快照</p>
    <template v-else>
      <div class="summary"><span role="status">{{ stale ? '状态已过期 · ' : '' }}{{ labels[task.state] }}</span><b>{{ task.progress_pct }}%</b></div>
      <progress :value="task.progress_pct" max="100" aria-label="Pi 任务总进度" />
      <p v-if="task.run_id" class="run-id">{{ task.run_id }}</p>
      <ol v-if="task.stages.length">
        <li v-for="stage in task.stages" :key="stage.id" :aria-current="stage.id === task.stage_id ? 'step' : undefined">{{ stage.label }}</li>
      </ol>
      <p v-if="task.detail">{{ task.detail }}</p>
    </template>
  </section>
</template>

<style scoped>
.pi-task{margin-bottom:12px;padding:12px 14px;border:1px solid #bac6bf;border-radius:6px;background:#fbfcfa;color:#28382f;font:12px/1.5 'Segoe UI',sans-serif;font-variant-numeric:tabular-nums;overflow-wrap:anywhere}
header,.summary{display:flex;justify-content:space-between;gap:8px;align-items:baseline}h2{font-size:15px;font-weight:650}header span,.run-id{color:#4c6557}.run-id{font-family:Consolas,monospace}p,.summary,ol{margin-top:6px}progress{width:100%;height:5px;display:block;accent-color:#367258}ol{padding-inline-start:1.5em;list-style:decimal}li{padding:2px 4px}li[aria-current=step]{background:#e8f2eb;color:#255a41;font-weight:650}[data-alert=true] .summary{color:#a13035}
</style>
