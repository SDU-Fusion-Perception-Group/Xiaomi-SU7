import { defineStore } from 'pinia'

export type Level = 'info' | 'warn' | 'error'
export interface LogEntry {
  id: number
  timestamp: string
  level: Level
  source: string
  message: string
}
export const LOG_LIMIT = 300

export function filterLogs(rows: readonly LogEntry[], level: Level | 'all', query: string) {
  const term = query.trim().toLowerCase()
  return rows.filter(row => (level === 'all' || row.level === level) &&
    (row.source.toLowerCase().includes(term) || row.message.toLowerCase().includes(term)))
}

export const useOps = defineStore('ops', {
  state: () => ({ entries: [] as LogEntry[], sequence: 0 }),
  getters: {
    errors: state => state.entries.filter(row => row.level === 'error'),
  },
  actions: {
    add(level: Level, source: string, message: string, tsMs = Date.now()) {
      // Out-of-range Unix times cannot be serialized as dates.
      const timestamp = new Date(tsMs <= 8.64e15 ? tsMs : Date.now()).toISOString()
      this.entries.push({ id: ++this.sequence, timestamp, level,
        source: source.slice(0, 96), message: message.slice(0, 4096) })
      if (this.entries.length > LOG_LIMIT) this.entries.shift()
    },
    jsonl() {
      return this.entries.map(row => JSON.stringify(row) + '\n').join('')
    },
    diagnose(input: string, mode: string, detected: number | null) {
      const cmd = input.trim().toLowerCase()
      switch (cmd) {
        case 'help': return 'help 帮助 · status 状态 · errors 最近错误 · logs 最近日志'
        case 'status': return '模式：' + mode + ' · 候选目标：' + (detected ?? '等待数据') + ' · 日志：' + this.entries.length
        case 'errors':
        case 'logs': {
          const rows = cmd === 'errors' ? this.errors : this.entries
          if (!rows.length) return cmd === 'errors' ? '暂无错误' : '暂无日志'
          return rows.slice(-10).reverse().map(row => '[' + row.level + '] ' + row.source + ' · ' + row.message).join('\n')
        }
        default: return '未知命令。输入 help 查看可用命令。'
      }
    },
  },
})
