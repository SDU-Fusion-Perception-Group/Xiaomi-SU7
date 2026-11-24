<script setup lang="ts">
import {Route,Pause,Play,Octagon,LockOpen} from 'lucide-vue-next'
type Mode='ready'|'running'|'paused'|'finished'|'estop'
defineProps<{mode:Mode;progress:number}>()
const emit=defineEmits<{start:[];pause:[];stop:[];unlock:[]}>()
const state={ready:'待执行',running:'执行中',paused:'已暂停',finished:'已完成',estop:'急停锁定'}
</script>
<template>
<header class="strip"><strong>工创地面站</strong><span class="tag">模拟驾驶</span><span class="status" role="status">{{state[mode]}}</span><button class="safety" :disabled="mode==='estop'" @click="emit('stop')"><Octagon/>模拟急停</button></header>
<section class="mission" aria-label="模拟路线任务"><small>任务控制</small><div class="title"><Route/><b>路线演练</b><span>{{progress}}%</span></div><progress :value="progress" max="100" aria-label="模拟路线完成度"/><button v-if="mode==='running'" @click="emit('pause')"><Pause/>暂停演练</button><button v-else-if="mode==='estop'" @click="emit('unlock')"><LockOpen/>解除锁定</button><button v-else @click="emit('start')"><Play/>{{mode==='finished'?'再次演练':mode==='paused'?'继续演练':'开始演练'}}</button></section>
</template>
<style scoped>
.strip,.mission{background:#fafcf9;color:#253c40;border:1px solid #bdcac5;font:13px 'Microsoft YaHei UI',sans-serif}.strip{position:absolute;inset:0 0 auto;height:52px;display:flex;gap:14px;align-items:center;padding:0 16px}.tag{background:#e4efea;color:#24594e;padding:3px 8px;border-radius:4px}.status{border-left:1px solid #cbd6d1;padding-left:14px}.mission{position:absolute;right:12px;top:278px;width:185px;padding:12px;border-radius:6px;box-shadow:0 2px 8px #40545216}.mission small{color:#5c706c}.title{display:flex;align-items:center;gap:6px;margin:7px 0}.title span{margin-left:auto;font:12px Consolas,monospace}.mission progress{display:block;width:100%;height:4px;accent-color:#327967;margin:10px 0}.strip button,.mission button{display:flex;align-items:center;justify-content:center;gap:6px;padding:7px;border-radius:4px}.safety{margin-left:auto;background:#fff1ed;color:#93382a;border:1px solid #e6afa4}.mission button{width:100%;background:#e1eee7;color:#1e5f50}.strip svg,.mission svg{width:16px;height:16px}@media(max-width:900px){.mission{top:124px;width:135px}.strip{gap:7px;padding:0 7px}.status{padding-left:6px}}@media(max-width:550px){.mission{top:auto;bottom:125px;right:8px;width:auto;padding:5px}.mission small,.mission .title,.mission progress{display:none}}
</style>