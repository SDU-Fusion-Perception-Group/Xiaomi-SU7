import {defineStore} from 'pinia'
// Metres, origin lower left. An illustrative lane-centre exercise, not Nav2 output.
export const route=[[2.25,2.25],[2.15,2.25],[2.15,1.2],[2.15,2.13],[1.2,2.13],[1.2,.28],[1.2,1.2],[.28,1.2],[1.2,1.2],[1.2,2.13],[2.15,2.13],[2.15,2.25],[2.25,2.25]] as const
const lengths=route.slice(1).map((p,i)=>Math.hypot(p[0]-route[i]![0],p[1]-route[i]![1]));const total=lengths.reduce((a,b)=>a+b,0)
export function poseAt(distance:number){let d=Math.min(total,Math.max(0,distance));for(let i=0;i<lengths.length;i++){let len=lengths[i]!;if(d<=len||i===lengths.length-1){const [x,y]=route[i]!,p=route[i+1]!,dx=p[0]-x,dy=p[1]-y;return {x:x+dx*d/len,y:y+dy*d/len,yaw:180 /* fixed mock body heading, independent of travel */}}d-=len}throw Error('empty route')}

// Motion and dwell use one simulation clock. Each stop is anchored to a route vertex.
const speed = .2 // m/s
const distanceTo = (vertex: number) => lengths.slice(0, vertex).reduce((a, b) => a + b, 0)
const plan = [
  { label: '前往二维码区', vertex: 2, hold: 0 },
  { label: '原地识别二维码', vertex: 2, hold: 3 },
  { label: '前往原料区', vertex: 5, hold: 0 },
  { label: '原地取料', vertex: 5, hold: 2 },
  { label: '搬运至暂存区', vertex: 7, hold: 0 },
  { label: '原地放料', vertex: 7, hold: 2 },
  { label: '返回启动区', vertex: route.length - 1, hold: 0 },
]
let endTime = 0
export const taskStages = plan.map(({ label, vertex, hold }, i) => {
  const from = distanceTo(plan[i - 1]?.vertex ?? 0)
  const to = distanceTo(vertex)
  const startTime = endTime
  endTime += hold || (to - from) / speed
  return { label, from, to, startTime, endTime }
})
const duration = endTime
type Mode = 'ready' | 'running' | 'paused' | 'finished' | 'estop'

export const useStation = defineStore('mock', {
  state: () => ({ mode: 'ready' as Mode, elapsed: 0, scan: true, grid: true, zoom: 1 }),
  getters: {
    stageIndex(s) {
      const index = taskStages.findIndex(stage => s.elapsed < stage.endTime)
      return index < 0 ? taskStages.length - 1 : index
    },
    stage(): (typeof taskStages)[number] { return taskStages[this.stageIndex]! },
    phase(): number {
      const { from, to, startTime } = this.stage
      return Math.min(to, from + (this.elapsed - startTime) * speed)
    },
    progress: s => Math.floor(100 * (s.elapsed / duration)),
    pose(): ReturnType<typeof poseAt> { return poseAt(this.phase) },
    x(): number { return this.pose.x },
    y(): number { return this.pose.y },
    yaw(): number { return this.pose.yaw },
  },
  actions: {
    start() {
      if (this.mode === 'estop' || this.mode === 'running') return
      if (this.elapsed >= duration) this.elapsed = 0
      this.mode = 'running'
    },
    pause() { if (this.mode === 'running') this.mode = 'paused' },
    stop() { this.mode = 'estop' },
    unlock() { if (this.mode === 'estop') this.mode = 'ready' },
    tick(dt: number) {
      if (this.mode !== 'running' || !Number.isFinite(dt) || dt <= 0) return
      this.elapsed = Math.min(duration, this.elapsed + Math.min(.15, dt))
      if (this.elapsed >= duration) this.mode = 'finished'
    },
    scale(v: number) { this.zoom = Math.min(2.6, Math.max(.65, v)) },
  },
})
