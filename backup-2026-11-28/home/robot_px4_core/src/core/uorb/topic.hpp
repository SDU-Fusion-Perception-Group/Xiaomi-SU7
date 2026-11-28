#pragma once
#include <atomic>
namespace robot {
template<class T> struct Topic {
  static_assert(std::atomic<int>::is_always_lock_free);
  struct Slot { std::atomic<int> busy{0}; T data{}; } slots[2];
  std::atomic<int> latest{-1};
};
template<class T> struct Publication {
  Topic<T>& topic;
  bool publish(const T& data) {
    int next=topic.latest.load(std::memory_order_relaxed)==0?1:0;
    auto& s=topic.slots[next]; int expected=0;
    if (!s.busy.compare_exchange_strong(expected,1,std::memory_order_acquire)) return false;
    s.data=data; topic.latest.store(next,std::memory_order_release);
    s.busy.store(0,std::memory_order_release); return true;
  }
};
template<class T> struct Subscription {
  Topic<T>& topic;
  bool copy(T& data) {
    int current=topic.latest.load(std::memory_order_acquire);
    if(current<0) return false;
    auto& s=topic.slots[current]; int expected=0;
    if(!s.busy.compare_exchange_strong(expected,1,std::memory_order_acquire)) return false;
    bool valid=topic.latest.load(std::memory_order_acquire)==current;
    if(valid) data=s.data;
    s.busy.store(0,std::memory_order_release); return valid;
  }
};
}
