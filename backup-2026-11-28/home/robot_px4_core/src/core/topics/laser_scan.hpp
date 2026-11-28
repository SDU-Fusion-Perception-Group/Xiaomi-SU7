#pragma once
#include <cstdint>
#include <limits>
namespace robot { struct LaserScan { std::uint64_t timestamp_us{}; double front_m=std::numeric_limits<double>::infinity(), left_m=front_m, right_m=front_m; }; }
