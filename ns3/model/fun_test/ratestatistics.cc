#include "ns3/simulator.h"
#include "ratestatistics.h"
namespace ns3{
//ms
static uint32_t RateObserverWindow=200;
void RateStatistics::OnSent(uint32_t len){
	uint32_t now=Simulator::Now().GetMilliSeconds();
	len_+=len;
	if(begin_==0){
		begin_=now;
	}
	if(now-begin_>RateObserverWindow){
		uint32_t duration=now-begin_;
		uint32_t sent=len_;
		uint32_t bps=sent*8*1000/duration;
		if(!m_rate.IsNull()){
			m_rate(bps);
		}
		Reset();
	}

}
void RateStatistics::Reset(){
	begin_=0;
	len_=0;
}
}
