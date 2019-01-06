#ifndef NS3_MPVIDEO_IDEALPACER_H_
#define NS3_MPVIDEO_IDEALPACER_H_
#include "net/third_party/quic/core/quic_bandwidth.h"
#include "net/third_party/quic/core/quic_time.h"
#include "idealcc.h"
namespace ns3{
class IdealPacingSender{
public:
	IdealPacingSender();
    void set_max_bps(uint32_t max_bps);
    void set_sender(IdealCC *sender){
    	sender_=sender;
    }
    void set_max_pacing_rate(quic::QuicBandwidth max_bandwidth){

    }
	void OnPacketSent(quic::QuicTime now,
			quic::QuicPacketNumber packet_number,
			quic::QuicByteCount bytes);
	void OnCongestionEvent(quic::QuicTime now,
			quic::QuicPacketNumber packet_number);
	quic::QuicTime::Delta TimeUntilSend(quic::QuicTime now) const;
private:
	quic::QuicTime ideal_next_packet_send_time_;
	quic::QuicTime::Delta alarm_granularity_;
	quic::QuicBandwidth usable_rate_;
	quic::QuicBandwidth pacing_rate_;
	IdealCC *sender_;
};
}




#endif /* NS3_MPVIDEO_IDEALPACER_H_ */
