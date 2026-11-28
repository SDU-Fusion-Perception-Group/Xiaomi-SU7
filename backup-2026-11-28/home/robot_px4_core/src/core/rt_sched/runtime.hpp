#pragma once
#include <pthread.h>
#include <time.h>
#include <cstdint>
namespace robot {
inline int bind_cpu(unsigned cpu) { cpu_set_t set; CPU_ZERO(&set); if(cpu>=CPU_SETSIZE) return -1; CPU_SET(cpu,&set); return pthread_setaffinity_np(pthread_self(),sizeof(set),&set); }
inline int set_priority(int policy,int priority) { sched_param p{}; p.sched_priority=priority; return pthread_setschedparam(pthread_self(),policy,&p); }
inline std::uint64_t monotonic_us() { timespec t{}; clock_gettime(CLOCK_MONOTONIC,&t); return std::uint64_t(t.tv_sec)*1000000+t.tv_nsec/1000; }
}
