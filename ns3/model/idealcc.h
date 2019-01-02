#ifndef NS3_MPVIDEO_IDEALCC_H_
#define NS3_MPVIDEO_IDEALCC_H_
#include "net/third_party/quic/core/quic_bandwidth.h"
namespace ns3{
class IdealCC{
public:
	IdealCC(uint32_t min_bps,void *observer):available_bw_(quic::QuicBandwidth::Zero()){
		min_bps_=min_bps;
        observer_=observer;
	}
	void set_ideal_rate(quic::QuicBandwidth bw){
		available_bw_=bw;
	}
	quic::QuicBandwidth GetReferenceRate(){
		return available_bw_;
	}
    bool ShouldSendProbePacket(){
        return false;
    }
    void UserDefinePeriod(uint32_t ms){
    }
private:
	quic::QuicBandwidth available_bw_;
    	uint32_t min_bps_;
    void *observer_{NULL};
};
}




#endif /* NS3_MPVIDEO_IDEALCC_H_ */
