const phases = ['idle', 'running', 'paused', 'estop', 'succeeded', 'failed', 'cancelled']
const text = (x, max) => typeof x === 'string' && x.length <= max && !!x.trim()
const id = x => typeof x === 'string' && /^[a-zA-Z0-9][a-zA-Z0-9_.:-]{0,63}$/.test(x)

export function validateTaskState(frame) {
  if (!frame || frame.schema_version !== 1 || frame.type !== 'task_state' ||
      !Number.isSafeInteger(frame.ts_ms) || frame.ts_ms < 0 ||
      !Number.isSafeInteger(frame.seq) || frame.seq < 0 || !phases.includes(frame.state) ||
      !Number.isInteger(frame.progress_pct) || frame.progress_pct < 0 || frame.progress_pct > 100 ||
      !Array.isArray(frame.stages) || frame.stages.length > 32 ||
      (frame.detail !== undefined && !text(frame.detail, 256))) throw Error('invalid task_state')
  const ids = new Set()
  for (const stage of frame.stages) {
    if (!id(stage?.id) || !text(stage.label, 80) || ids.has(stage.id)) throw Error('invalid task_state stages')
    ids.add(stage.id)
  }
  if (frame.state === 'idle') {
    if (frame.run_id !== null || frame.stage_id !== null || frame.progress_pct !== 0 || ids.size) throw Error('invalid idle task_state')
  } else if (!id(frame.run_id) || !ids.size ||
    (frame.state === 'succeeded' ? frame.stage_id !== null || frame.progress_pct !== 100 :
      !ids.has(frame.stage_id) || frame.progress_pct === 100)) throw Error('invalid task_state run, stage or progress')
  return frame
}
