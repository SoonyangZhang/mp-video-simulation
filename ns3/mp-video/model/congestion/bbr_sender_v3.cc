#include "bbr_sender_v3.h"
#include <iostream>
#include <assert.h>
namespace quic{
const float kDefaultHighGain = 2.0f;
const float kDecreaseGain=0.75;
const float kStartupGrowthTarget = 1.25;
const int64_t kRoundTripsWithoutGrowthBeforeExitingStartup = 3;
const float kBandwidthLossFactor=0.9;//0.95;//0.9
//const float KDelayIncreaseFactor=
const int64_t kContinuousBandwidthLoss=1;
const float kSimilarMinRttThreshold = 1.125;
const uint64_t kBandwidthWindowSize=10;
const uint32_t kInitialCongestionWindowPackets = 10;
const QuicByteCount kDefaultSegmentSize= 1000;
// The time after which the current min_rtt value expires.
const QuicTime::Delta kMinRttExpiry = QuicTime::Delta::FromSeconds(10);
// in compatible with traditional tcp,
//cwnd increase 1 every rtt.
const uint32_t kPacketIncreaseSize=1000;
const uint32_t kBandWidthIncrease=5000;
const float kCongestionBackoff=0.8;
const float kSelfInflictQueueBackoff=0.9;
// if cur_rtt_-min_rtt_>kTolerableDelayOffset congested;
const uint64_t kTolerableDelayOffset=50;
const uint32_t kSmoothRttNum=90;
const uint32_t kSmoothRttDen=100;
const float kDelayBackoffGain=0.9;
const uint64_t kDelayBackoffWindow=10;
MyBbrSenderV3::MyBbrSenderV3(uint64_t min_bps,BandwidthObserver *observer)
:min_bw_(QuicBandwidth::FromBitsPerSecond(min_bps))
,observer_(observer)
,min_rtt_(QuicTime::Delta::Zero())
,s_rtt_(QuicTime::Delta::Zero())
,min_rtt_timestamp_(QuicTime::Zero())
,min_rtt_in_decrease_(QuicTime::Delta::Infinite())
,min_rtt_timestamp_in_decrease_(QuicTime::Zero())
,min_rtt_record_(QuicTime::Delta::Zero())
,exit_probe_rtt_at_(QuicTime::Zero())
,max_bw_in_increase_(QuicBandwidth::Zero())
,pacing_rate_(QuicBandwidth::Zero())
,max_bandwidth_(kBandwidthWindowSize,QuicBandwidth::Zero(),0)
,high_gain_(kDefaultHighGain)
,drain_gain_{1.0/kDefaultHighGain}
,num_startup_rtts_{kRoundTripsWithoutGrowthBeforeExitingStartup}
,num_bw_decrease_rtts_(kContinuousBandwidthLoss)
,bandwidth_at_last_round_(QuicBandwidth::Zero())
,initial_congestion_window_(kInitialCongestionWindowPackets*kDefaultSegmentSize){
	EnterStartUpMode();
}
MyBbrSenderV3::~MyBbrSenderV3(){}
QuicBandwidth MyBbrSenderV3::PaceRate(){
	if(pacing_rate_==QuicBandwidth::Zero()){
		return high_gain_*min_bw_;
	}
	return pacing_rate_;
}
QuicBandwidth MyBbrSenderV3::GetReferenceRate(){
	if(pacing_rate_==QuicBandwidth::Zero()){
		return min_bw_;
	}
	return pacing_rate_;
}
QuicBandwidth MyBbrSenderV3::BandwidthEstimate(){
	return max_bandwidth_.GetBest();
}
bool MyBbrSenderV3::ShouldSendProbePacket(){
	return true;
}
void MyBbrSenderV3::OnPacketSent(QuicTime event_time,
		QuicPacketNumber packet_number,
		QuicByteCount bytes){
	if(packet_number<=last_sent_packet_){
		return;
	}
	last_sent_packet_=packet_number;
	sampler_.OnPacketSent(event_time,packet_number,bytes,
			bytes_in_flight_);
	bytes_in_flight_+=bytes;
	AddPacketInfo(event_time,packet_number,bytes);
}
void MyBbrSenderV3::OnAck(QuicTime event_time,
		QuicPacketNumber packet_number){
	if(packet_number<=last_ack_seq_){
		return;
	}
	if(last_ack_seq_<packet_number){
		QuicPacketNumber i=last_ack_seq_+1;
		for(;i<packet_number;i++){
			sampler_.OnPacketLost(i);
		}
	}
	last_ack_seq_=packet_number;
	UpdateRttAndInflight(event_time,packet_number);
	int64_t current_round=round_trip_count_;
	bool is_round_update=false;
	is_round_update=UpdateRoundTripCounter(last_ack_seq_);
	// Handle logic specific to STARTUP and DRAIN modes.
	QuicBandwidth bw=sampler_.OnPacketAcknowledged(event_time,packet_number,is_round_update);
	if(current_round==0){
		CalculatePacingRate();
		return ;
	}
	if(is_round_update){
		max_bandwidth_.Update(bw,current_round);
		if(mode_==ST_INCREASE){
			UpdateMaxBw();
		}
	}
	if (is_round_update && !is_at_full_bandwidth_) {
	   CheckIfFullBandwidthReached();
	}
	MaybeExitStartupOrDrain(event_time);
	bool min_rtt_expired=false;
	min_rtt_expired=event_time>(min_rtt_timestamp_ + kMinRttExpiry);
    bool is_congested=false;
	if(mode_==ST_INCREASE){
		if(is_round_update){
			is_congested=CheckIfCongestion();
		}
	}
    //min_rtt_expired=false;//for disable 10s expire, then, other flow will not get rate;
    MaybeEnterOrExitDecrease(event_time,min_rtt_expired,is_congested,current_round);
	CalculatePacingRate();
}
void MyBbrSenderV3::EnterStartUpMode(){
	mode_=ST_START;
	pacing_gain_=high_gain_;
}
void MyBbrSenderV3::CheckIfFullBandwidthReached(){
	QuicBandwidth target = bandwidth_at_last_round_ * kStartupGrowthTarget;
	  if (BandwidthEstimate() >= target) {
	    bandwidth_at_last_round_ = BandwidthEstimate();
	    rounds_without_bandwidth_gain_ = 0;
	    return;
	  }

	  rounds_without_bandwidth_gain_++;
	  if ((rounds_without_bandwidth_gain_ >= num_startup_rtts_)) {
	    is_at_full_bandwidth_ = true;
	  }
}
bool MyBbrSenderV3::CheckIfCongestion(){
	bool congestion=false;
	if(s_rtt_==QuicTime::Delta::Zero()||min_rtt_==QuicTime::Delta::Zero()){
		return congestion;
	}
	QuicTime::Delta offset=QuicTime::Delta::FromMilliseconds(kTolerableDelayOffset);
	if(s_rtt_-min_rtt_>offset){
		congestion=true;
	}
	return congestion;
}
void MyBbrSenderV3::MaybeExitStartupOrDrain(QuicTime now){
	if(mode_==ST_START&&is_at_full_bandwidth_){
		mode_=ST_DRAIN;
		pacing_gain_=drain_gain_;
	}
	if(mode_==ST_DRAIN){
		if(bytes_in_flight_<=GetTargetCongestionWindow(1.0)){
			EnterIncreaseMode(now);
		}
	}
}
void MyBbrSenderV3::EnterIncreaseMode(QuicTime now){
	mode_=ST_INCREASE;
	pacing_gain_=1.0;
}
void MyBbrSenderV3::MaybeEnterOrExitDecrease(QuicTime now,
                              bool min_rtt_expired,bool is_congested,int64_t round){
	if(mode_!=ST_DECREASE&&(min_rtt_expired||is_congested)){
		mode_=ST_DECREASE;
		min_rtt_record_=min_rtt_;
		exit_probe_rtt_at_ = QuicTime::Zero();
		QuicBandwidth best=BandwidthEstimate();
		QuicBandwidth target=QuicBandwidth::Zero();
		max_bw_in_increase_=QuicBandwidth::Zero();
        sending_rate_=QuicBandwidth::Zero();
        min_rtt_in_decrease_=QuicTime::Delta::Infinite();
		if(round-last_backoff_count_<kDelayBackoffWindow){
			pacing_gain_=kDelayBackoffGain;
		}else{
			pacing_gain_ =kDecreaseGain;
			if(is_congested){
				target=best*kCongestionBackoff;
			}else{
				target=best*kSelfInflictQueueBackoff;
			}
	        dynamic_congestion_back_off_=0.0;
			max_bandwidth_.Reset(QuicBandwidth::Zero(),0);
			max_bandwidth_.Update(target,round);
		}
		last_backoff_count_=round;
	}
	if(mode_==ST_DECREASE){
		if(exit_probe_rtt_at_==QuicTime::Zero()){
			exit_probe_rtt_at_=now;
		}
		bool excess_drained=false;
		if(bytes_in_flight_<=GetTargetInflightInDecrease(1.0)){
			excess_drained=true;
		}
		if(/*((now-exit_probe_rtt_at_)>4*min_rtt_record_)||*/excess_drained){
			if(s_rtt_==QuicTime::Delta::Zero()){
				min_rtt_=min_rtt_record_;
				min_rtt_timestamp_=now;
			}else{
				min_rtt_=s_rtt_;
				min_rtt_timestamp_=now;
			}
			EnterIncreaseMode(now);
		}
	}
}
QuicTime::Delta MyBbrSenderV3::GetMinRtt() {
  return !min_rtt_.IsZero() ? min_rtt_ :QuicTime::Delta::FromMilliseconds(200);
}
QuicByteCount MyBbrSenderV3::GetTargetCongestionWindow(float gain){
  QuicByteCount bdp = GetMinRtt() * BandwidthEstimate();
  QuicByteCount congestion_window = gain * bdp;

  // BDP estimate will be zero if no bandwidth samples are available yet.
  if (congestion_window == 0) {
    congestion_window = gain * initial_congestion_window_;
  }

  return congestion_window;
}
QuicByteCount MyBbrSenderV3::GetTargetInflightInDecrease(float gain){
	  QuicByteCount bdp = min_rtt_record_* BandwidthEstimate();
	  QuicByteCount congestion_window = gain * bdp;

	  // BDP estimate will be zero if no bandwidth samples are available yet.
	  if (congestion_window == 0) {
	    congestion_window = gain * initial_congestion_window_;
	  }

	  return congestion_window;
}
void MyBbrSenderV3::CalculatePacingRate(){
	QuicBandwidth bw=BandwidthEstimate();
	QuicBandwidth last_rate=pacing_rate_;
	pacing_rate_=pacing_gain_*bw;
	if(pacing_rate_==QuicBandwidth::Zero()){
		pacing_rate_=QuicBandwidth::FromBytesAndTimeDelta(
		        initial_congestion_window_,GetMinRtt());
	}
	if(mode_==ST_INCREASE){
		uint64_t rtt=GetMinRtt().ToMilliseconds();
		uint64_t increase_bps=kPacketIncreaseSize*1000*8/rtt;
        uint64_t base_bps=pacing_rate_.ToKBitsPerSecond()*1000;
		uint64_t bps=base_bps+increase_bps;
        //uint64_t bps=pacing_rate_.ToKBitsPerSecond()*1000+kBandWidthIncrease;
        sending_rate_=QuicBandwidth::FromBitsPerSecond(base_bps);
		pacing_rate_=QuicBandwidth::FromBitsPerSecond(bps);
	}
	if(pacing_rate_!=last_rate){
		std::string state=GetStateString();
		uint64_t bps=pacing_rate_.ToKBitsPerSecond()*1000;
		if(!trace_state_cb_.IsNull()){
			trace_state_cb_(bps,state);
		}
		if(observer_){
			observer_->OnBandwidthUpdate();
		}
	}
}
bool MyBbrSenderV3::UpdateRoundTripCounter(QuicPacketNumber last_acked_packet){
	  if (last_acked_packet > current_round_trip_end_) {
	    round_trip_count_++;
	    current_round_trip_end_ = last_sent_packet_;
	    return true;
	  }
	  return false;
}
void MyBbrSenderV3::AddPacketInfo(QuicTime now,QuicPacketNumber packet_number,QuicByteCount payload){
	std::shared_ptr<PerPacket> packet(new PerPacket(now,payload));
	sent_packets_map_.insert(std::make_pair(packet_number,packet));
}
void MyBbrSenderV3::UpdateRttAndInflight(QuicTime now,
		QuicPacketNumber packet_number){
	auto it=sent_packets_map_.find(packet_number);
	if(it!=sent_packets_map_.end()){
		std::shared_ptr<PerPacket> packet=it->second;
		QuicTime::Delta rtt=now-packet->sent_ts;
		if(min_rtt_==QuicTime::Delta::Zero()){
			min_rtt_=rtt;
			min_rtt_timestamp_=now;
		}else {
			if(rtt<min_rtt_){
				min_rtt_=rtt;
				min_rtt_timestamp_=now;
			}
			if(rtt>min_rtt_&&rtt<=min_rtt_*kSimilarMinRttThreshold){
				min_rtt_timestamp_=now;
			}
		}
		if(s_rtt_==QuicTime::Delta::Zero()){
			s_rtt_=rtt;
		}else{
			int64_t old_ms=s_rtt_.ToMilliseconds();
			int64_t new_ms=rtt.ToMilliseconds();
			int64_t smooth_ms=((kSmoothRttDen-kSmoothRttNum)*old_ms+kSmoothRttNum*new_ms)/kSmoothRttDen;
			s_rtt_=QuicTime::Delta::FromMilliseconds(smooth_ms);
		}
		if(mode_==ST_DECREASE){
			if(rtt<=min_rtt_in_decrease_){
				min_rtt_in_decrease_=rtt;
				min_rtt_timestamp_in_decrease_=now;
			}
		}
	}
	while(!sent_packets_map_.empty()){
		auto it=sent_packets_map_.begin();
		if(it->first<=packet_number){
			std::shared_ptr<PerPacket> packet=it->second;
			bytes_in_flight_-=packet->bytes;
			sent_packets_map_.erase(it);
		}else{
			break;
		}
	}
}
void MyBbrSenderV3::UpdateMaxBw(){
	QuicBandwidth bw=BandwidthEstimate();
	if(bw>max_bw_in_increase_){
		max_bw_in_increase_=bw;
	}
}
std::string MyBbrSenderV3::GetStateString(){
	if(mode_==ST_START){
		return std::string("start");
	}
	if(mode_==ST_DRAIN){
		return std::string("drain");
	}
	if(mode_==ST_INCREASE){
		return std::string("increase");
	}
	if(mode_==ST_DECREASE){
		return std::string("decrease");
	}
    return std::string("null");
}
void MyBbrSenderV3::PrintDebugInfo(uint64_t bps,std::string state){
	if(!trace_state_cb_.IsNull()){
		trace_state_cb_(bps,state);
	}
}
}
