import socket,sys,time,struct,concurrent.futures
H='10.42.87.1'; P=48736; B=b'x'*65536
def exact(s,n):
 d=b''
 while len(d)<n:
  x=s.recv(n-len(d))
  if not x: raise EOFError()
  d+=x
 return d
def send(s,duration):
 t=time.monotonic(); n=0
 while time.monotonic()-t<duration:s.sendall(B);n+=len(B)
 s.shutdown(socket.SHUT_WR);return n
def recv(s):
 n=0;t=time.monotonic()
 while True:
  d=s.recv(262144)
  if not d:return n,time.monotonic()-t
  n+=len(d)
def handle(s):
 with s:
  s.settimeout(14);m=exact(s,1)
  if m==b'U':
   n,t=recv(s);s.sendall(struct.pack('!Qd',n,t))
  else:send(s,5)
def test(m):
 with socket.create_connection((H,P),4) as s:
  s.settimeout(14);s.sendall(m)
  if m==b'U':send(s,5);return struct.unpack('!Qd',exact(s,16))
  return recv(s)
if sys.argv[1]=='server':
 with socket.socket() as s,concurrent.futures.ThreadPoolExecutor(12) as pool:
  s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1);s.bind((H,P));s.listen();print('READY',flush=True)
  while True:c,_=s.accept();pool.submit(handle,c)
else:
 for n in (1,4):
  for m in (b'D',b'U'):
   for run in (1,2,3):
    t=time.monotonic()
    with concurrent.futures.ThreadPoolExecutor(n) as p: a=list(p.map(test,[m]*n))
    print(m.decode(),n,run,round(sum(x[0] for x in a)*8/max(x[1] for x in a)/1e6,2),a,flush=True)
