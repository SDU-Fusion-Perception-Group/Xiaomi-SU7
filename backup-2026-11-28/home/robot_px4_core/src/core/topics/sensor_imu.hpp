#pragma once
#include <array>
#include <cstdint>
namespace robot { struct SensorImu { std::uint64_t timestamp_us{}, sequence{}; std::array<double,3> acceleration_m_s2{}, angular_velocity_rad_s{}, euler_rad{}; }; }
