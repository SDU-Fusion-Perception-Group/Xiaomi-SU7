<script setup lang="ts">
import type { TaskSnapshot } from '../task-protocol.mjs'
import PiTaskStatus from './PiTaskStatus.vue'
defineProps<{
  tasks: ReadonlyMap<string | null, { snapshot: TaskSnapshot; stale: boolean }>
  hiddenRuns: ReadonlySet<string | null>
}>()
const emit = defineEmits<{ visibility: [id: string | null, visible: boolean] }>()
function change(id: string | null, event: Event) {
  emit('visibility', id, (event.target as HTMLInputElement).checked)
}
</script>

<template>
  <section aria-label="Pi 任务监视">
    <fieldset v-if="tasks.size && !tasks.has(null)" class="channels">
      <legend>显示任务</legend>
      <label v-for="[id] in tasks" :key="id ?? ''">
        <input type="checkbox" :checked="!hiddenRuns.has(id)" @change="change(id, $event)" />
        <span>{{ id }}</span>
      </label>
    </fieldset>
    <PiTaskStatus v-if="!tasks.size" :task="null" :stale="false" />
    <p v-else-if="hiddenRuns.size === tasks.size" class="channels" role="status">未选中任务，后台仍在接收上报。</p>
    <template v-for="[id, entry] in tasks" :key="id ?? ''">
      <PiTaskStatus v-if="!hiddenRuns.has(id)" :task="entry.snapshot" :stale="entry.stale" />
    </template>
  </section>
</template>

<style scoped>
.channels{min-width:0;max-height:9rem;overflow:auto;margin:0 0 10px;padding:5px 9px;border:1px solid #bac6bf;border-radius:6px;color:#28382f;font-size:12px}
label{display:flex;gap:8px;align-items:baseline;margin:4px 0;cursor:pointer}span{overflow-wrap:anywhere}input{accent-color:#367258;flex:none}
</style>
