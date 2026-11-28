#pragma once
#include "core/uorb/topic.hpp"
#include "core/topics/sensor_imu.hpp"
#include "core/topics/laser_scan.hpp"
namespace robot { void run_imu(const std::atomic<bool>& stop, Topic<SensorImu>&); void run_lidar(const std::atomic<bool>& stop, Topic<LaserScan>&); }
