#include "ns_quic_time.h"
#include "ns3/simulator.h"
namespace ns3{
quic::QuicTime Ns3QuicTime::Now(){
uint32_t ms=Simulator::Now().GetMilliSeconds();
quic::QuicTime time=quic::QuicTime::Zero() + quic::QuicTime::Delta::FromMilliseconds(ms);
return time;
}
}



