#include "drivers/hwt905_parser.hpp"
#include <cstdlib>
#include <iostream>
int main() {
  robot::Hwt905Parser p; int count=0;
  std::array<std::uint8_t,11> f{0x55,0x51,1,0,255,255,3,0,0,0,0};
  for(unsigned i=0;i<10;++i) f[10]+=f[i];
  auto feed=[&](unsigned char b){p.feed(b,[&](unsigned type,auto data){if(type!=0x51 || data!=std::array<int,3>{1,-1,3}) std::exit(1);++count;});};
  for(auto x:f) feed(x); for(auto x:f) feed(x);
  if(count!=2) return 2;
  for(unsigned i=0;i<5;++i) feed(f[i]); if(count!=2) return 3;
  for(unsigned i=5;i<11;++i) feed(f[i]); if(count!=3) return 4;
  auto bad=f; ++bad[10]; for(auto x:bad) feed(x); if(count!=3) return 5;
  for(unsigned i=0;i<6;++i) feed(f[i]); for(auto x:f) feed(x);
  if(count!=4) return 6;
  std::cout<<"HWT905 parser: concatenation, partial input, invalid checksum, and resynchronization passed\n";
}
