<script setup lang="ts">
import { computed, ref } from 'vue'
import { Terminal, ChevronDown, Download, Copy } from 'lucide-vue-next'
import { filterLogs, LOG_LIMIT, useOps, type Level } from '../stores/ops'

const props = defineProps<{ mode: string; detected: number | null }>()
const ops = useOps()
const open = ref(false)
const views = { cli: '终端', logs: '日志', errors: '问题', ssh: 'SSH' } as const
const view = ref<keyof typeof views>('cli')
const level = ref<Level | 'all'>('all')
const query = ref('')
const command = ref('')
const output = ref('输入 help 查看可用命令。')
const rows = computed(() => view.value === 'errors' ? ops.errors.slice().reverse() : filterLogs(ops.entries, level.value, query.value).reverse())
function run() {
  output.value = '> ' + command.value + '\n' + ops.diagnose(command.value, props.mode, props.detected)
  command.value = ''
}
function download(event: MouseEvent) {
  const link = event.currentTarget as HTMLAnchorElement
  const url = URL.createObjectURL(new Blob([ops.jsonl()], { type: 'application/x-ndjson;charset=utf-8' }))
  link.href = url
  window.setTimeout(() => URL.revokeObjectURL(url), 1000)
}

function selectView(key: keyof typeof views) {
  open.value = view.value === key ? !open.value : true
  view.value = key
}

const host = ref('')
const login = ref('')
const port = ref('22')
const feedback = ref('')
const preview = ref<HTMLInputElement | null>(null)
const sshCommand = computed(() => {
  // Limit each argument to a single literal token in system terminals.
  if (!/^[a-z0-9][a-z0-9.-]*$/i.test(host.value) ||
      (login.value && !/^[a-z0-9_][a-z0-9_.-]*$/i.test(login.value)) ||
      !/^[1-9][0-9]{0,4}$/.test(port.value) || Number(port.value) > 65535) return ''
  return 'ssh -p ' + port.value + ' ' + (login.value ? login.value + '@' : '') + host.value
})
async function copyCommand() {
  const text = sshCommand.value
  if (!text) return
  try {
    await navigator.clipboard.writeText(text)
    if (sshCommand.value === text) feedback.value = '已复制，请粘贴到系统终端运行。'
  } catch {
    preview.value?.select()
    feedback.value = '剪贴板不可用，请选中命令后手动复制。'
  }
}
</script>

<template>
  <aside class="ops" :class="{ expanded: open }" aria-label="操作台" @keydown.space.stop>
    <nav aria-label="操作台视图">
      <Terminal class="mark" aria-hidden="true"/>
      <button v-for="(label, key) in views" :key="key" :aria-expanded="open && view === key" aria-controls="ops-panel" @click="selectView(key)">
        {{ label }}<span v-if="key === 'errors'" class="badge">{{ ops.errors.length }}</span>
      </button>
      <button class="toggle" :aria-label="open ? '折叠操作台' : '展开操作台'" :aria-expanded="open" aria-controls="ops-panel" @click="open = !open"><ChevronDown :class="{ up: !open }"/></button>
    </nav>
    <section v-show="open" id="ops-panel" class="panel" :aria-label="views[view]">
      <template v-if="view === 'cli'">
        <small>本地只读终端</small>
        <pre role="status">{{ output }}</pre>
        <form @submit.prevent="run"><input v-model="command" maxlength="80" aria-label="只读诊断命令" placeholder="输入只读命令，如 status" spellcheck="false" @keydown.space.stop/><button :disabled="!command.trim()">运行</button></form>

      </template>
      <template v-else-if="view === 'ssh'">
        <p>复制命令到系统终端连接。本面板不建立连接，也不收集密码。</p>
        <div class="fields" @input="feedback = ''">
          <label>主机<input v-model.trim="host" placeholder="主机名或 IPv4" maxlength="253" spellcheck="false"/></label>
          <label>用户<input v-model.trim="login" placeholder="可选" maxlength="64" spellcheck="false"/></label>
          <label>端口<input v-model.trim="port" inputmode="numeric" maxlength="5"/></label>
        </div>
        <div class="controls"><input ref="preview" readonly aria-label="SSH 命令" :value="sshCommand" placeholder="填写有效主机、用户名及端口（1–65535）"/><button :disabled="!sshCommand" @click="copyCommand"><Copy/>复制命令</button></div>
        <small role="status">{{ feedback }}</small>
      </template>
      <template v-else>
        <div v-if="view === 'logs'" class="controls">
          <select v-model="level" aria-label="日志级别"><option value="all">全部级别</option><option>info</option><option>warn</option><option>error</option></select>
          <input v-model="query" type="search" aria-label="搜索来源或消息" placeholder="搜索来源或消息"/>
          <a href="#" download="station-logs.jsonl" title="导出当前保留的全部日志" @click="download"><Download/>导出 JSONL</a>
        </div>
        <ol v-if="rows.length" class="entries"><li v-for="row in rows" :key="row.id"><time :datetime="row.timestamp" :title="row.timestamp">{{ row.timestamp.slice(11, 19) }}Z</time> <b :class="row.level">{{ row.level }}</b> {{ row.source }}<p>{{ row.message }}</p></li></ol>
        <p v-else class="empty">{{ view === 'errors' ? '暂无错误记录' : ops.entries.length ? '没有匹配的日志' : '等待运行日志' }}</p>
        <footer>显示 {{ rows.length }} 条 · 会话内保留最近 {{ LOG_LIMIT }} 条</footer>
      </template>
    </section>
  </aside>
</template>

<style scoped>
.ops{box-sizing:border-box;width:100%;min-width:0;height:36px;flex:none;display:flex;flex-direction:column;border-top:1px solid #b9c3c0;background:#f9fbfa;color:#263932;color-scheme:light;font:12px/1.45 'Segoe UI',sans-serif}
.ops.expanded{height:190px}
nav,.controls,form,.fields,button,a,label{display:flex;align-items:center;gap:7px}
nav{height:35px;flex:none;gap:0;padding:0 8px;background:#eef2f0;border-bottom:1px solid #d5deda}
nav button{height:100%;padding:0 10px;border-bottom:2px solid transparent;white-space:nowrap}
nav button[aria-expanded=true]:not(.toggle){color:#155f50;border-color:#257b68;font-weight:650}
button:hover:not(:disabled),a:hover{background:#dfe8e3}.mark{margin:0 5px}.toggle{margin-left:auto}.up{transform:rotate(180deg)}
.ops svg{width:14px;height:14px;flex:none}.badge{font:11px/1.4 Consolas,monospace;padding:0 5px;background:#dce5e0;border-radius:8px}
.panel{display:flex;flex:1;min-height:0;flex-direction:column;align-items:stretch;gap:6px;padding:8px 12px;overflow:auto}
input,select,.controls button,form button,.controls a{background:#fff;border:1px solid #c0cbc5;border-radius:3px;padding:3px 7px}
.controls,form,.fields{flex:none}input{min-width:0;flex:1}label{flex:1;min-width:0}label:last-child{flex:0 1 85px}button:disabled{opacity:.5}.controls a,.controls button{white-space:nowrap}
pre,.entries,.empty{flex:1;min-height:24px;overflow:auto;overscroll-behavior:contain}.entries li{padding:2px 0;border-bottom:1px solid #e0e6e2;overflow-wrap:anywhere}.entries p{display:inline;margin-left:8px}.entries p,pre{white-space:pre-wrap;overflow-wrap:anywhere}
pre,input[readonly],time,b,footer{font-family:Consolas,monospace}pre{padding:4px 8px;background:#eff3f0}.empty,small,time,footer{color:#53655b}small,footer{font-size:11px}.error{color:#a02f30}.warn{color:#875900}.info{color:#29604c}
@media(max-width:650px){nav button{padding:0 7px}.fields{flex-wrap:wrap}.fields label{flex:1 1 100px}.panel{padding:6px 8px}.controls{gap:3px}}
</style>
