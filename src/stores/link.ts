import { ref, shallowReactive, shallowRef } from 'vue'
import { defineStore } from 'pinia'
import { useOps } from './ops.ts'
import { useScope } from './scope.ts'
import type { ArenaScan } from './perception'
import { validateTaskState, type TaskSnapshot } from '../task-protocol.mjs'
export const useLink=defineStore('pi-link',()=>{
 const state=ref('未连接'),scan=shallowRef<ArenaScan|null>(null),ops=useOps(),scope=useScope(); let socket:WebSocket|null=null,timer=0
 const tasks = shallowReactive(new Map<string | null, { snapshot: TaskSnapshot; stale: boolean }>())
 const hiddenRuns = shallowReactive(new Set<string | null>())
 const taskTimers = new Map<string | null, ReturnType<typeof setTimeout>>()
 let lastTaskSeq = -1
 function dropTask(id: string | null) {
   clearTimeout(taskTimers.get(id))
   taskTimers.delete(id)
   tasks.delete(id)
   hiddenRuns.delete(id)
 }
 function resetTask() {
   for (const id of tasks.keys()) dropTask(id)
   lastTaskSeq = -1
 }
 function setTaskVisible(id: string | null, visible: boolean) {
   if (!tasks.has(id)) return
   if (visible) hiddenRuns.delete(id)
   else hiddenRuns.add(id)
 }
 function ingestTask(frame: unknown) {
   const snapshot = validateTaskState(frame)
   if (snapshot.seq <= lastTaskSeq) return
   lastTaskSeq = snapshot.seq
   const id = snapshot.run_id
   if (id === null) {
     // A session-wide idle frame is not an update to any named run.
     if (tasks.size > Number(tasks.has(null))) return
   } else if (tasks.has(null)) dropTask(null)
   if (!tasks.has(id) && tasks.size >= 32) {
     const entries = [...tasks.values()]
     const candidates = entries.filter(entry => entry.stale || ['succeeded', 'failed', 'cancelled'].includes(entry.snapshot.state))
     const oldest = (candidates.length ? candidates : entries).reduce((a, b) => a.snapshot.seq < b.snapshot.seq ? a : b)
     dropTask(oldest.snapshot.run_id)
   }
   tasks.set(id, { snapshot, stale: false })
   clearTimeout(taskTimers.get(id))
   taskTimers.set(id, setTimeout(() => {
     taskTimers.delete(id)
     tasks.set(id, { snapshot, stale: true })
   }, 3500))
 }
 function close(){resetTask();window.clearTimeout(timer);const old=socket;socket=null;old?.close();scan.value=null;state.value='未连接'}
 function connect(address:string){close();scope.reset();try{
 const url=new URL(address);if(!['ws:','wss:'].includes(url.protocol)||url.pathname!='/ws'||url.username||url.password||url.search||url.hash)throw Error('请填写 ws(s)://主机:端口/ws');const ws=new WebSocket(url);socket=ws;state.value='连接中';timer=window.setTimeout(()=>{if(socket===ws){close();ops.add('error','链路','握手超时，请检查 Pi 地址和端口')}},8000);
 ws.onopen=()=>{if(ws!==socket)return;clearTimeout(timer);state.value='只读在线';ops.add('info','链路','Pi 遥测已连接')};ws.onclose=e=>{if(ws!==socket)return;clearTimeout(timer);scan.value=null;resetTask();socket=null;state.value='未连接';ops.add(e.wasClean?'info':'error','链路','连接关闭，代码 '+e.code)};
 ws.onmessage=e=>{if(socket!==ws)return;try{if(typeof e.data!=='string'||e.data.length>262144)throw Error('帧过大或不是文本');const f=JSON.parse(e.data);if(f?.schema_version!==1||!Number.isSafeInteger(f.ts_ms)||f.ts_ms<0)throw Error('版本或时间戳错误');if(f.type==='task_state'){ingestTask(f);return}if(f.type==='telemetry'){scope.ingest(f);return}if(f.type==='event'&&['info','warn','error'].includes(f.level)&&typeof f.source==='string'&&typeof f.message==='string'&&f.source.trim()&&f.message.trim()){ops.add(f.level,'Pi / '+f.source,f.message,f.ts_ms);return}if(f.type!=='scan_lite'||f.frame_id!=='map'||!Array.isArray(f.points)||f.points.length>4096||!f.points.every((p:unknown)=>Array.isArray(p)&&p.length===2&&p.every(Number.isFinite)))throw Error('无效的遥测消息');if(Math.abs(Date.now()-f.ts_ms)>1500)throw Error('扫描已过期，请核对两端时钟');scan.value={frame_id:'map',ts_ms:f.ts_ms,points:f.points.map(([x,y]:number[])=>({x,y}))}}catch(err){close();ops.add('error','协议',String(err))}};
 }catch(err){state.value='未连接';ops.add('error','链路',String(err))}}
 return {state,scan,tasks,hiddenRuns,setTaskVisible,connect,close}
})
