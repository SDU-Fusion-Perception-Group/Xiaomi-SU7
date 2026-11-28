import socket,sys,time,struct
H='10.42.87.1'; P=48737; SIZE=1200; SECS=4

def send(s,peer,mb):
 start=time.perf_counter(); step=SIZE*8/(mb*1e6); i=0; nmax=int(SECS/step)
 while i<nmax:
  delay=start+i*step-time.perf_counter()
  if delay>.001:time.sleep(delay-.0005)
  if delay>0:continue
  s.sendto(struct.pack('!I',i)+bytes(SIZE-4),peer);i+=1
 duration=time.perf_counter()-start
 for _ in range(3):s.sendto(b'E'+struct.pack('!Id',i,duration),peer);time.sleep(.03)
 return i,duration

def recv(s):
 seen=set(); dup=ooo=0; high=-1; first=prev=None; jitter=0
 while True:
  b,p=s.recvfrom(2048); now=time.perf_counter()
  if len(b)==SIZE:
   i=struct.unpack_from('!I',b)[0]
   if i in seen:dup+=1;continue
   if i<high:ooo+=1
   high=max(high,i);seen.add(i)
   if prev is not None:jitter+=(abs((now-prev[0])-(i-prev[1])*step)-jitter)/16
   else:first=now
   prev=now,i
  elif len(b)==13 and b[:1]==b'E':
   n,d=struct.unpack('!Id',b[1:]);return f'sent={n} rx={len(seen)} loss_pct={(n-len(seen))*100/n:.3f} tx_Mbps={n*SIZE*8/d/1e6:.2f} rx_Mbps={len(seen)*SIZE*8/d/1e6:.2f} jitter_ms={jitter*1000:.3f} dup={dup} reorder={ooo}',p

def drain(s):
 s.setblocking(False)
 try:
  while True:s.recvfrom(2048)
 except BlockingIOError:pass
 finally:s.settimeout(9)

with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as s:
 s.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,1048576)
 if sys.argv[1]=='server':
  s.bind((H,P));print('UDP_READY',flush=True)
  while True:
   s.settimeout(None);b,p=s.recvfrom(2048)
   if len(b)!=5:continue
   mb=struct.unpack('!I',b[1:])[0];step=SIZE*8/(mb*1e6);s.settimeout(9)
   if b[:1]==b'D':send(s,p,mb)
   elif b[:1]==b'U':
    s.sendto(b'ACK',p); result,_=recv(s);s.sendto(result.encode(),p);print('U',mb,result,flush=True)
 else:
  p=(H,P)
  for mb in (10,30,50,75,100):
   for m in (b'D',b'U'):
    drain(s);step=SIZE*8/(mb*1e6);s.sendto(m+struct.pack('!I',mb),p)
    if m==b'D':result,_=recv(s)
    else:
     assert s.recvfrom(2048)[0]==b'ACK';send(s,p,mb);result=s.recvfrom(2048)[0].decode()
    print(m.decode(),mb,result,flush=True);time.sleep(.2)