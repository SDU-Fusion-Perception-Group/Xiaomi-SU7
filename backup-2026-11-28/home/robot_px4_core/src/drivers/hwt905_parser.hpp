#pragma once
#include <array>
#include <cstdint>
#include <algorithm>
namespace robot {
class Hwt905Parser {
  std::array<std::uint8_t,11> frame{}; unsigned used=0;
  static int value(const std::uint8_t* p) { int x=p[0]+256*p[1]; return x>=32768?x-65536:x; }
public:
  template<class Emit> void feed(std::uint8_t byte,Emit emit) {
    if(!used && byte!=0x55) return;
    frame[used++]=byte;
    if(used==2 && (byte<0x51 || byte>0x53)) { used=byte==0x55?1:0; return; }
    if(used<11) return;
    unsigned sum=0; for(unsigned i=0;i<10;++i) sum+=frame[i];
    if((sum&255)==frame[10]) { used=0; emit(frame[1],std::array<int,3>{value(&frame[2]),value(&frame[4]),value(&frame[6])}); return; }
    unsigned i=1; while(i<11 && (frame[i]!=0x55 || (i+1<11 && (frame[i+1]<0x51 || frame[i+1]>0x53)))) ++i;
    std::move(frame.begin()+i,frame.end(),frame.begin()); used=11-i;
  }
};
}
