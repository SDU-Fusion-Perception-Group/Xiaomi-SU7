import test from 'node:test'
import assert from 'node:assert/strict'
import {createPinia,setActivePinia} from 'pinia'
import {useStation,route,poseAt} from '../src/stores/station.ts'
const total=route.slice(1).reduce((d,p,i)=>d+Math.hypot(p[0]-route[i][0],p[1]-route[i][1]),0)
test('reference trajectory centre stays in field and outside four restricted squares',()=>{for(let d=0;d<=total;d+=.005){const {x,y}=poseAt(d);assert.ok(x>=0&&x<=2.4&&y>=0&&y<=2.4);for(const left of [.55,1.4])for(const bottom of [.55,1.4])assert.ok(!(x>left&&x<left+.45&&y>bottom&&y<bottom+.45))}})
test('mock stop is latched, unlock does not move vehicle',()=>{setActivePinia(createPinia());const s=useStation();s.start();s.tick(.1);assert.ok(s.phase>0);s.stop();const phase=s.phase;s.start();s.tick(.1);assert.equal(s.phase,phase);s.unlock();s.tick(.1);assert.equal(s.phase,phase);s.start();s.tick(.1);assert.ok(s.phase>phase)})
test('pause holds position; resume continues from there',()=>{setActivePinia(createPinia());const s=useStation();s.start();s.tick(.1);s.pause();const d=s.phase;s.tick(.1);assert.equal(s.mode,'paused');assert.equal(s.phase,d);s.start();s.tick(.1);assert.ok(s.phase>d)})
test('finish parks in starting zone and does not restart itself',()=>{setActivePinia(createPinia());const s=useStation();s.start();for(let i=0;i<10000;i++)s.tick(.1);assert.equal(s.mode,'finished');assert.ok(Math.abs(s.x-2.25)<1e-8&&Math.abs(s.y-2.25)<1e-8)})

test('ordered stages dwell at QR, pickup and delivery landmarks, then return', () => {
  setActivePinia(createPinia())
  const s = useStation(), seen = [], counts = new Map()
  const stops = new Map([['原地识别二维码', [2.15,1.2]], ['原地取料', [1.2,.28]], ['原地放料', [.28,1.2]]])
  s.start()
  let lastProgress = 0
  for (let n=0; n<1000 && s.mode === 'running'; n++) {
    const name = s.stage.label
    if (seen.at(-1) !== name) seen.push(name)
    const point = stops.get(name)
    if (point) {
      assert.ok(Math.hypot(s.x-point[0],s.y-point[1]) < 1e-8, name)
      counts.set(name,(counts.get(name) ?? 0)+1)
    }
    s.tick(.1)
    assert.ok(s.progress >= lastProgress)
    lastProgress = s.progress
  }
  assert.deepEqual(seen,['前往二维码区','原地识别二维码','前往原料区','原地取料','搬运至暂存区','原地放料','返回启动区'])
  for (const name of stops.keys()) assert.ok(counts.get(name) >= 19, name)
  assert.equal(s.mode,'finished')
  assert.equal(s.progress,100)
  s.start()
  assert.equal(s.stageIndex,0)
  assert.equal(s.progress,0)
  assert.equal(s.phase,0)
})

test('pause and latched estop freeze the dwell clock; unlock alone never resumes it', () => {
  setActivePinia(createPinia())
  const s = useStation()
  s.start()
  for(let i=0;i<60;i++) s.tick(.1)
  assert.equal(s.stage.label,'原地识别二维码')
  const snapshot = () => [s.elapsed,s.progress,s.stageIndex,s.phase,s.x,s.y]
  const held = snapshot()
  s.pause()
  for(let i=0;i<50;i++) s.tick(.1)
  assert.deepEqual(snapshot(),held)
  s.stop(); s.start()
  assert.equal(s.mode,'estop')
  for(let i=0;i<50;i++) s.tick(.1)
  assert.deepEqual(snapshot(),held)
  s.unlock(); s.tick(.1)
  assert.deepEqual(snapshot(),held)
  s.start(); s.tick(.1)
  assert.ok(s.elapsed > held[0]); assert.equal(s.phase,held[3])
})
