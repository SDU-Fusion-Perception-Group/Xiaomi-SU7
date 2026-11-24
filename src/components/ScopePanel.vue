<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref } from 'vue'
import TracePlot from './TracePlot.vue'
import { Activity, X } from 'lucide-vue-next'
import { channels, FRESH_MS, useScope, type Channel } from '../stores/scope'
import { useLink } from '../stores/link'
const scope = useScope(), link = useLink()
const now = ref(Date.now())
let timer: number
onMounted(() => { timer = window.setInterval(() => now.value = Date.now(), 250) })
onUnmounted(() => window.clearInterval(timer))
const shortNames={gyro:'陀螺 Z',motor:'电机转速',fused:'融合航向',fused_x:'融合 X',fused_y:'融合 Y'}
const choices = Object.keys(channels) as Channel[]
const missing = computed(() => choices.filter(ch => !scope.tabs.includes(ch)))
const samples = computed(() => scope.active ? scope.visible(scope.active, now.value) : [])
const latest = computed(() => samples.value[samples.value.length - 1])
const status = computed(() => {
  if (link.state !== '只读在线') return link.state === '连接中' ? '连接中' : '链路未连接'
  if (!scope.active || !scope.history[scope.active].length) return '等待首帧'
  return latest.value && now.value - latest.value.ts_ms <= FRESH_MS ? '实时' : '测量断流'
})
</script>

<template>
  <section class="scope glass" aria-label="遥测示波器">
    <header><Activity :size="18"/><h2>遥测示波器</h2><small>Pi / WS</small></header>
    <nav aria-label="测量标签">
      <button v-for="ch in scope.tabs" :key="ch" :aria-pressed="ch === scope.active" @click="scope.open(ch)">{{ shortNames[ch] }}</button>
      <button v-for="ch in missing" :key="'add-' + ch" :aria-label="'添加 ' + ch" @click="scope.open(ch)">+ {{ shortNames[ch] }}</button>
      <button v-if="scope.active" :aria-label="'移除 ' + scope.active" @click="scope.remove(scope.active)"><X :size="14"/></button>
    </nav>
    <template v-if="scope.active">
      <div class="line"><strong>{{ channels[scope.active].name }}</strong><small role="status" :class="{ live: status === '实时' }">{{ status }}</small></div>
      <div class="reading mono">{{ status === '实时' ? latest?.value.toFixed(2) : '—' }} <small>{{ channels[scope.active].unit }}</small></div>
      <div class="line"><label>历史窗口 <select v-model.number="scope.windowMs"><option v-for="s in [5,10,30]" :key="s" :value="1000*s">{{ s }} s</option></select></label><small>{{ samples.length }} 点</small></div>
      <ul v-if="samples.length" aria-label="最近采样" class="mono"><li v-for="p in samples.slice(-4).reverse()" :key="p.ts_ms"><span>{{ p.ts_ms }} ms</span><span>{{ p.value }} {{ channels[scope.active].unit }}</span></li></ul>
      <p v-else>窗口内暂无采样</p>
      <TracePlot :samples="samples" :end="now" :window-ms="scope.windowMs"/>
    </template>
    <p v-else>添加测量标签以查看采样。</p>
  </section>
</template>

<style scoped>
.scope{display:grid;align-content:start;gap:.65rem;padding:.9rem;min-width:0;font-size:.78rem;color:#dfebf0}
header,.line,nav,li{display:flex;align-items:center;gap:.45rem;flex-wrap:wrap}header svg,.live{color:#56dcbb}h2{font-size:.95rem;font-weight:650}header small{margin-inline-start:auto}small,p,li{color:#9eafbf}nav button,select{padding:.22rem .45rem;border:1px solid #354453;border-radius:.35rem;background:#172333;color:inherit}nav button[aria-pressed=true]{background:#163943;color:#81e8db;border-color:#487974}.line,li{justify-content:space-between}.reading{font-size:1.65rem}.reading small{font-size:.85rem}li{font-size:.7rem;padding:.14rem 0}ul{border-top:1px solid #354453}
.scope{color:#293e3b;background:#fbfcfa;padding:10px}small,p,li{color:#52675f}nav button,select{background:#f7faf6;color:#355448;border-color:#becec4}nav button[aria-pressed=true]{color:#175a4c;background:#e1efe6}header svg,.live{color:#23785f}
</style>
