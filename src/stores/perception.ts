// Metres in field frame. Only the mock range generator knows target locations.
export type Point = {x:number,y:number}
// Two cylinders for this exercise only. Sample once per session, not once per scan.
const fixtures:Point[]=[{x:1.12+Math.random()*.16,y:.65+Math.random()*1.05},{x:1.65+Math.random()*.4,y:1.12+Math.random()*.16}]
export interface ArenaScan {frame_id:'map'; ts_ms:number; points:Point[]}
export function scanCandidates(frame:ArenaScan,now=Date.now()) {
 if(frame.frame_id!=='map'||!Number.isFinite(frame.ts_ms)||Math.abs(now-frame.ts_ms)>1500||!Array.isArray(frame.points)||frame.points.length>4096)return {points:[],objects:[]};
 const points=frame.points.filter(p=>p&&Number.isFinite(p.x)&&Number.isFinite(p.y));return {points,objects:detect(points)}
}
export function mockScan(origin:Point,yaw:number,targets=fixtures):Point[]{
 const out:Point[]=[]
 for(let i=0;i<1440;i++){
  const a=yaw*Math.PI/180+i*Math.PI/720,dx=Math.cos(a),dy=Math.sin(a)
  let t=Math.min(Math.abs(dx)<1e-8?Infinity:((dx>0?2.4:0)-origin.x)/dx,Math.abs(dy)<1e-8?Infinity:((dy>0?2.4:0)-origin.y)/dy)
  for(const c of targets){const x=c.x-origin.x,y=c.y-origin.y,along=x*dx+y*dy,offset=x*dy-y*dx,disc=.025**2-offset**2;if(along>0&&disc>=0)t=Math.min(t,along-Math.sqrt(disc))}
  if(t>0)out.push({x:origin.x+t*dx,y:origin.y+t*dy})
 }return out
}
// Cluster interior returns; broad surfaces and isolated returns are not labelled cylinders.
export function detect(points:Point[]){const groups:Point[][]=[];for(const p of points){if(!(p.x>.035&&p.x<2.365&&p.y>.035&&p.y<2.365))continue;const g=groups.find(g=>g.some(q=>Math.hypot(p.x-q.x,p.y-q.y)<.027));if(g)g.push(p);else groups.push([p])}return groups.flatMap(g=>{if(g.length<3)return [];const xs=g.map(p=>p.x),ys=g.map(p=>p.y),w=Math.max(...xs)-Math.min(...xs),h=Math.max(...ys)-Math.min(...ys);if(Math.hypot(w,h)<.016||Math.hypot(w,h)>.07)return [];return [{x:xs.reduce((a,b)=>a+b)/g.length,y:ys.reduce((a,b)=>a+b)/g.length}]})}
