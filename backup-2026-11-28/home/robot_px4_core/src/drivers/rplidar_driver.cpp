#include "drivers/drivers.hpp"
#include "core/rt_sched/runtime.hpp"
#include "sl_lidar_driver.h"
#include <algorithm>
#include <memory>
#include <cstdio>
#include <vector>
namespace robot {
void run_lidar(const std::atomic<bool>& stop,Topic<LaserScan>& topic) {
  if(int e=bind_cpu(0)) std::fprintf(stderr,"Lidar CPU affinity: %d\n",e);
  auto ch=sl::createSerialPortChannel("/dev/rplidar",256000);
  auto drv=sl::createLidarDriver();
  std::unique_ptr<sl::IChannel> channel(ch?*ch:nullptr);
  std::unique_ptr<sl::ILidarDriver> lidar(drv?*drv:nullptr);
  if(!channel || !lidar || SL_IS_FAIL(lidar->connect(channel.get()))) { std::fprintf(stderr,"RPLIDAR connect failed\n");return; }
  lidar->setMotorSpeed();
  if(SL_IS_FAIL(lidar->startScan(false,true))) { std::fprintf(stderr,"RPLIDAR startScan failed\n");lidar->setMotorSpeed(0);return; }
  std::vector<sl_lidar_response_measurement_node_hq_t> nodes(8192);
  Publication<LaserScan> pub{topic};
  while(!stop) {
    size_t n=nodes.size(); if(SL_IS_FAIL(lidar->grabScanDataHq(nodes.data(),n,300))) continue;
    LaserScan s{}; s.timestamp_us=monotonic_us();
    for(size_t i=0;i<n;++i) {
      const auto& p=nodes[i]; if(!p.dist_mm_q2 || !p.quality) continue;
      const double a=p.angle_z_q14*(90.0/16384), d=p.dist_mm_q2/4000.0;
      if(a<=15 || a>=345) s.front_m=std::min(s.front_m,d);
      else if(a>=255 && a<=285) s.left_m=std::min(s.left_m,d);
      else if(a>=75 && a<=105) s.right_m=std::min(s.right_m,d);
    }
    pub.publish(s);
  }
  lidar->stop(); lidar->setMotorSpeed(0);
}
}
