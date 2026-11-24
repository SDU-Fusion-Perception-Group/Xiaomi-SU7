import { ref, shallowReactive } from 'vue'
import { defineStore } from 'pinia'

export const channels = {
  gyro: { name: '陀螺仪 · Z 轴角速度', unit: '°/s' },
  motor: { name: '电机 · 反馈转速', unit: 'rpm' },
  fused_x: {name:'融合位置 · X',unit:'m'}, fused_y: {name:'融合位置 · Y',unit:'m'},
  fused: { name: '融合 · 航向角', unit: '°' },
} as const
export type Channel = keyof typeof channels
export type Sample = { ts_ms: number; value: number }
export const FRESH_MS = 1500
export const LIMIT = 2048
const RETAIN_MS = 30000

export const useScope = defineStore('scope', () => {
  const tabs = ref<Channel[]>(['gyro'])
  const active = ref<Channel | null>('gyro')
  const windowMs = ref(10000)
  const history = shallowReactive<Record<Channel, Sample[]>>({ gyro: [], motor: [], fused: [], fused_x: [], fused_y: [] })
  function open(channel: Channel) {
    if (!tabs.value.includes(channel)) tabs.value.push(channel)
    active.value = channel
  }
  function remove(channel: Channel) {
    const i = tabs.value.indexOf(channel)
    if (i < 0) return
    tabs.value.splice(i, 1)
    if (active.value === channel) active.value = tabs.value[i] ?? tabs.value[i - 1] ?? null
  }
  function ingest(f: {channel: unknown; unit: unknown; value: unknown; ts_ms: number}, now = Date.now()) {
    const ch = f.channel
    if (ch !== 'gyro' && ch !== 'motor' && ch !== 'fused' && ch !== 'fused_x' && ch !== 'fused_y') throw Error('未知测量通道')
    if (f.unit !== channels[ch].unit || typeof f.value !== 'number' || !Number.isFinite(f.value)) throw Error('测量单位或数值错误')
    if (!Number.isSafeInteger(f.ts_ms) || f.ts_ms < 0 || Math.abs(now - f.ts_ms) > FRESH_MS) throw Error('测量时间戳无效或过期，请检查对时')
    const arr = history[ch]
    if (arr.length && arr[arr.length - 1].ts_ms >= f.ts_ms) return
    let i = Math.max(0, arr.length + 1 - LIMIT)
    while (i < arr.length && arr[i].ts_ms < f.ts_ms - RETAIN_MS) i++
    history[ch] = [...arr.slice(i), { ts_ms: f.ts_ms, value: f.value }]
  }
  function visible(channel: Channel, now: number) {
    return history[channel].filter(p => p.ts_ms >= now - windowMs.value)
  }
  function reset() { history.gyro = []; history.motor = []; history.fused = []; history.fused_x=[]; history.fused_y=[] }
  return { tabs, active, windowMs, history, open, remove, ingest, visible, reset }
})
