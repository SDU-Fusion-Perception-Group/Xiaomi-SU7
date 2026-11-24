<script setup lang="ts">
import { Play, Pause, LockOpen, Check } from 'lucide-vue-next'
import { taskStages, useStation } from '../stores/station'
const station = useStation()
const status = { ready: '待命', running: '执行中', paused: '已暂停', estop: '急停锁定', finished: '已完成' }
</script>

<template>
  <section class="task-flow" aria-label="模拟任务流程" :data-mode="station.mode">
    <header><h2>任务流程</h2><span class="source">模拟</span></header>
    <div class="summary"><span role="status">{{ status[station.mode] }}</span><b>{{ station.progress }}%</b></div>
    <progress :value="station.progress" max="100" aria-label="模拟任务总进度" />
    <ol>
      <li v-for="(step,i) in taskStages" :key="step.label" :class="{ done: i < station.stageIndex || station.progress === 100 }" :aria-current="i === station.stageIndex && station.progress < 100 ? 'step' : undefined">
        <span class="index"><Check v-if="i < station.stageIndex || station.progress === 100" :size="12"/><template v-else>{{ i+1 }}</template></span>{{ step.label }}
      </li>
    </ol>
    <button v-if="station.mode === 'estop'" type="button" @click="station.unlock()"><LockOpen/>解除急停</button>
    <button v-else-if="station.mode === 'running'" type="button" @click="station.pause()"><Pause/>暂停任务</button>
    <button v-else type="button" @click="station.start()"><Play/>{{ station.progress === 100 ? '重新开始' : station.elapsed ? '继续任务' : '启动任务' }}</button>
  </section>
</template>

<style scoped>
.task-flow{min-width:0;padding:12px 14px;background:#fbfcfa;color:#28382f;border:1px solid #bac6bf;border-radius:6px;font-size:12px;font-variant-numeric:tabular-nums}
header,.summary,li,button,.index{display:flex;align-items:center}.task-flow header,.summary{justify-content:space-between}h2{font-size:15px;font-weight:650}.source{padding:1px 6px;border:1px solid #b6c8bd;border-radius:4px;color:#40604c;font-size:11px}
.summary{margin-top:8px;color:#4c6557}progress{display:block;width:100%;height:4px;accent-color:#367258;border:0;border-radius:2px;overflow:hidden}progress::-webkit-progress-bar{background:#dde6df}progress::-webkit-progress-value{background:#367258}
ol{list-style:none;margin:8px 0;padding:0}li{gap:8px;padding:3px 6px;color:#58695e}li[aria-current=step]{color:#255a41;background:#e8f2eb;font-weight:650;border-radius:3px}.index{width:16px;justify-content:center;font-family:Consolas,monospace}.done .index{color:#367258}
button{width:100%;gap:6px;justify-content:center;background:#315d49;color:white;border-radius:4px;padding:6px;cursor:pointer}button svg{width:14px;height:14px}button:focus-visible{outline:2px solid #3c8065;outline-offset:2px}[data-mode=estop] .summary{color:#a13035}[data-mode=estop] button{background:#843d41}
</style>
