#include "idealpacer.h"
using namespace quic;
const float bw_gain=1;//1.2;
namespace ns3{
IdealPacingSender::IdealPacingSender()
:ideal_next_packet_send_time_(QuicTime::Zero())
,alarm_granularity_(QuicTime::Delta::FromMilliseconds(1))
,usable_rate_(QuicBandwidth::Zero())
,pacing_rate_(QuicBandwidth::Zero()){
}
void IdealPacingSender::set_max_bps(uint32_t max_bps){
    usable_rate_=QuicBandwidth::FromBitsPerSecond(max_bps);
	pacing_rate_=usable_rate_*bw_gain;
	sender_->set_ideal_rate(usable_rate_);
}
void IdealPacingSender::OnPacketSent(quic::QuicTime now,
		quic::QuicPacketNumber packet_number,
		quic::QuicByteCount bytes){
	QuicTime::Delta delay=pacing_rate_.TransferTime(bytes);
    ideal_next_packet_send_time_ =
        std::max(ideal_next_packet_send_time_ + delay, now + delay);
}
void IdealPacingSender::OnCongestionEvent(quic::QuicTime now,
		quic::QuicPacketNumber packet_number){

}
quic::QuicTime::Delta IdealPacingSender::TimeUntilSend(quic::QuicTime now) const{
	if(ideal_next_packet_send_time_>now+alarm_granularity_){
		return ideal_next_packet_send_time_ - now;
	}
	return QuicTime::Delta::Zero();
}
}




