#include "drivers/drivers.hpp"
#include "drivers/hwt905_parser.hpp"
#include "core/rt_sched/runtime.hpp"
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>
namespace robot {
void run_imu(const std::atomic<bool>& stop,Topic<SensorImu>& topic) {
  if(int e=bind_cpu(0)) std::fprintf(stderr,"IMU CPU affinity: %d\n",e);
  int fd=open("/dev/imu_hwt905",O_RDWR|O_NOCTTY|O_NONBLOCK|O_CLOEXEC);
  if(fd<0) { std::perror("IMU open"); return; }
  Publication<SensorImu> pub{topic}; SensorImu m{}; unsigned locked=0;
  constexpr double pi=3.141592653589793;
  while(!stop) for(unsigned i=0;i<2 && !stop;++i) {
    if(locked && locked!=i+1) continue;
    termios t{}; if(tcgetattr(fd,&t)<0) {close(fd);return;}
    cfmakeraw(&t); t.c_cflag|=CLOCAL|CREAD; t.c_cflag&=~(CRTSCTS|CSTOPB);
    cfsetspeed(&t,i?B230400:B115200);
    if(tcsetattr(fd,TCSAFLUSH,&t)<0) continue;
    Hwt905Parser parser; unsigned mask=0;
    auto last=monotonic_us();
    while(!stop && monotonic_us()-last<1000000) {
      pollfd p{fd,POLLIN,0}; int result=poll(&p,1,100);
      if(result<0 && errno!=EINTR) {close(fd);return;}
      if(p.revents&(POLLERR|POLLHUP|POLLNVAL)) {close(fd);return;}
      unsigned char bytes[256]; int n=(p.revents&POLLIN)?read(fd,bytes,sizeof(bytes)):0;
      for(int j=0;j<n;++j) parser.feed(bytes[j],[&](unsigned kind,auto raw){
        last=monotonic_us(); mask|=1u<<(kind-0x51);
        for(unsigned k=0;k<3;++k) {
          if(kind==0x51) m.acceleration_m_s2[k]=raw[k]*(16*9.80665/32768);
          if(kind==0x52) m.angular_velocity_rad_s[k]=raw[k]*(2000*pi/180/32768);
          if(kind==0x53) m.euler_rad[k]=raw[k]*(pi/32768);
        }
        if(mask==7) { locked=i+1; m.timestamp_us=last; ++m.sequence; pub.publish(m); mask=0; }
      });
    }
    locked=0;
  }
  close(fd);
}
}
