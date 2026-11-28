#include "drivers/drivers.hpp"
#include "core/rt_sched/runtime.hpp"
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <thread>
#include <cmath>
namespace { volatile std::sig_atomic_t quit=0; void on_signal(int){quit=1;} }
int main(int argc,char** argv) {
  using namespace robot;
  std::signal(SIGINT,on_signal);std::signal(SIGTERM,on_signal);
  const double seconds=argc>1?std::atof(argv[1]):0;
  std::atomic<bool> stop{false}; Topic<SensorImu> it; Topic<LaserScan> lt;
  std::thread imu(run_imu,std::cref(stop),std::ref(it)), lidar(run_lidar,std::cref(stop),std::ref(lt));
  Subscription<SensorImu> is{it}; Subscription<LaserScan> ls{lt};
  SensorImu m{}; LaserScan s{}; auto start=monotonic_us(), last=start; std::uint64_t seq=0;
  auto dist=[](double d){return std::isfinite(d)?std::to_string(d):std::string("--");};
  auto next=std::chrono::steady_clock::now();
  while(!quit && (seconds<=0 || monotonic_us()-start<seconds*1000000)) {
    is.copy(m);ls.copy(s); auto now=monotonic_us();
    std::cout<<std::fixed<<std::setprecision(2)<<"IMU ";
    if(m.timestamp_us && now-m.timestamp_us<1000000) std::cout<<(m.sequence-seq)*1e6/double(now-last+1)<<" Hz yaw="<<m.euler_rad[2]<<" rad wz="<<m.angular_velocity_rad_s[2]<<" rad/s"; else std::cout<<"no data";
    if(!s.timestamp_us || now-s.timestamp_us>1000000) s=LaserScan{};
    std::cout<<" | lidar F/L/R m: "<<dist(s.front_m)<<" / "<<dist(s.left_m)<<" / "<<dist(s.right_m)<<std::endl;
    seq=m.sequence;last=now; next+=std::chrono::milliseconds(100);std::this_thread::sleep_until(next);
  }
  stop=true;imu.join();lidar.join();
}
