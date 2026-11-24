export type TaskState = 'idle' | 'running' | 'paused' | 'estop' | 'succeeded' | 'failed' | 'cancelled'
export interface TaskSnapshot {
  schema_version: 1
  type: 'task_state'
  ts_ms: number
  seq: number
  state: TaskState
  run_id: string | null
  stage_id: string | null
  progress_pct: number
  stages: { id: string; label: string }[]
  detail?: string
}
export function validateTaskState(frame: unknown): TaskSnapshot
