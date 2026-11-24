<script setup lang="ts">
import {onMounted,onUnmounted,ref,watch} from 'vue'
import {Minus,Plus,Grid2x2,Radar,RotateCcw,Route} from 'lucide-vue-next'
import {useStation} from './stores/station'
import MapCanvas from './components/MapCanvas.vue'
import TaskFlow from './components/TaskFlow.vue'
import PiTaskMonitor from './components/PiTaskMonitor.vue'
import ScopePanel from './components/ScopePanel.vue'
const view=ref('地图')
import OpsConsole from './components/OpsConsole.vue'
import {useOps} from './stores/ops'
import {useLink} from './stores/link'
const s=useStation(),ops=useOps(),link=useLink(),address=ref('ws://raspberrypi.local:8765/ws'),settings=ref(false)
watch(()=>s.mode,(mode)=>ops.add(mode==='estop'?'warn':'info','模拟底盘',({estop:'急停锁定',ready:'待命',running:'路线演练中',paused:'路线已暂停',finished:'路线已完成'})[mode]))
const showRoute=ref(false)
const obstacles=ref(true), detected=ref<number|null>(null)
const fps=ref(0),p95=ref(0)
let timer=0,prev=0,elapsed=0,intervals:number[]=[]
function frame(t:number){
 if(prev){const dt=t-prev;s.tick(dt/1000);elapsed+=dt;intervals.push(dt)
 if(elapsed>=2000){fps.value=Math.round(intervals.length*1000/elapsed);intervals.sort((a,b)=>a-b);p95.value=Math.round(intervals[Math.floor(intervals.length*.95)]??0);elapsed=0;intervals=[]}}
 prev=t;timer=requestAnimationFrame(frame)
}
function visibility(){cancelAnimationFrame(timer);prev=0;elapsed=0;intervals=[];fps.value=0;if(!document.hidden)timer=requestAnimationFrame(frame)}
function key(e:KeyboardEvent){if(e.code==='Space'&&!(e.target instanceof Element && e.target.closest('input,button,select,textarea,[contenteditable]'))){e.preventDefault();s.stop()}}
onMounted(()=>{ops.add('info','工作站','会话开始；位姿与控制为模拟源');visibility();document.addEventListener('visibilitychange',visibility);window.addEventListener('keydown',key)})
onUnmounted(()=>{link.close();cancelAnimationFrame(timer);document.removeEventListener('visibilitychange',visibility);window.removeEventListener('keydown',key)})
</script>
<template><main class="desk" :class="{locked:s.mode==='estop'}">
<header class="top"><b>工创地面站</b><span class="badge">模拟驾驶</span>    <div class="linkbar"><button @click="settings=!settings" :aria-expanded="settings">Pi 接入 · {{link.state}}</button><form v-if="settings" @submit.prevent="link.connect(address)"><label>只读遥测地址<input v-model="address" aria-label="Pi WebSocket 地址" /></label><button type="submit">连接</button><button type="button" @click="link.close();ops.add('info','链路','手动断开')">断开</button></form></div><button class="stop" @click="s.stop" :disabled="s.mode==='estop'">{{s.mode==='estop'?'模拟急停已锁定':'模拟急停'}}</button></header>
<div class="switcher"><button v-for="tab in ['任务','地图','测量']" :key="tab" :aria-pressed="tab===view" @click="view=tab">{{tab}}</button></div>
<div class="workspace" :data-view="view"><aside class="left"><PiTaskMonitor v-if="link.state==='只读在线'" :tasks="link.tasks" :hidden-runs="link.hiddenRuns" @visibility="link.setTaskVisible"/><TaskFlow/><section class="pose"><small>模拟车位姿 · ENU</small><p>X {{s.x.toFixed(2)}} m　Y {{s.y.toFixed(2)}} m</p><p>航向 {{s.yaw.toFixed(1)}}°</p></section></aside>
<section class="arena"><div class="maphead"><strong>智能搬运初赛</strong>    <nav class="map-tools flex items-center" aria-label="地图工具">
      <button title="显示或隐藏模拟路线（非比赛规定路线）" :aria-pressed="showRoute" @click="showRoute=!showRoute"><Route/> 模拟路线</button>
      <button title="显示或隐藏检测结果" :aria-pressed="obstacles" @click="obstacles=!obstacles">障碍</button>
      <button title="切换点云" aria-label="切换点云" :aria-pressed="s.scan" @click="s.scan=!s.scan"><Radar/></button>
      <button title="切换网格" aria-label="切换网格" :aria-pressed="s.grid" @click="s.grid=!s.grid"><Grid2x2/></button>
      <button title="缩小" aria-label="缩小" @click="s.scale(s.zoom/1.15)"><Minus/></button>
      <button title="重置缩放" aria-label="重置缩放" @click="s.scale(1)"><RotateCcw/></button>
      <button title="放大" aria-label="放大" @click="s.scale(s.zoom*1.15)"><Plus/></button>
    </nav></div><div class="canvas">    <MapCanvas :scan-frame="link.scan" :obstacles="obstacles" @detection="detected=$event" :x="s.x" :y="s.y" :yaw="s.yaw" :scan="s.scan" :grid="s.grid" :show-route="showRoute" :zoom="s.zoom" @zoom-change="s.scale" /></div><div class="mapfoot"><span>{{detected===null?'等待实时点云':'障碍候选 '+detected}}</span><span>{{fps||'—'}} FPS · P95 {{p95||'—'}} ms</span></div></section>
<aside class="right"><ScopePanel/></aside></div><OpsConsole :mode="s.mode" :detected="detected" /></main></template>
<style scoped>
.desk{position:fixed;inset:0;display:flex;flex-direction:column;overflow:hidden;background:#eff2ef;color:#273e40;font:13px 'Segoe UI','Microsoft YaHei UI',sans-serif}.top,.maphead,.mapfoot,.switcher{display:flex;align-items:center;gap:10px;background:#fafcf9;padding:7px 12px;border-bottom:1px solid #c5cfca}.top{position:relative;z-index:4;min-height:46px}.top b{font-size:16px}.badge,.stop,.linkbar button,.switcher button{padding:4px 9px;border-radius:4px}.badge{background:#deeee6;color:#28604f}.linkbar{margin-left:auto}.linkbar button{border:1px solid #bdcbc5}.linkbar form{position:absolute;right:12px;top:100%;background:#fff;padding:12px;border:1px solid #b9c9c2;border-radius:5px}.linkbar input{display:block;border:1px solid #b5c6c0;padding:5px;margin-bottom:6px;width:230px}.stop{color:#8e3629;background:#fae7e1;border:1px solid #dea99c}.workspace{flex:1;min-height:0;display:grid;grid-template-columns:clamp(185px,18vw,250px) minmax(0,1fr) clamp(238px,23vw,350px)}.left,.right{padding:12px;min-width:0;overflow:auto;background:#f6f8f5}.arena{min-width:0;display:flex;flex-direction:column;border-inline:1px solid #c3cdc7}.canvas{position:relative;flex:1;min-height:0}.maphead{justify-content:space-between;flex-wrap:wrap}.map-tools,.map-tools button{display:flex;align-items:center;gap:3px}.map-tools button{padding:5px;color:#47615a;border-radius:4px}.map-tools svg{width:15px;height:15px}.map-tools button[aria-pressed=true],.switcher button[aria-pressed=true]{background:#d7ebe2;color:#195c4a}.mapfoot{justify-content:space-between;font:11px Consolas,monospace;color:#546760}.pose{margin-top:14px;border-top:1px solid #d2dbd5;padding:12px 0;font:13px/1.7 Consolas,monospace}.switcher{display:none}.locked{outline:3px solid #bc4a3c;outline-offset:-3px}@media(max-width:850px){.switcher{display:flex;padding:3px 8px}.workspace{display:block;position:relative}.workspace>*{height:100%}.workspace[data-view=地图] :is(.left,.right),.workspace[data-view=任务] :is(.arena,.right),.workspace[data-view=测量] :is(.arena,.left){display:none}.maphead{gap:2px}}@media(max-width:450px){.top{gap:4px;padding:5px}.top b{font-size:13px}}
</style>