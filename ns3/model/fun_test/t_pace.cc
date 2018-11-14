#include "t_pace.h"
#include "ns3/mpcommon.h"
#include "ns3/simulator.h"
#include "rtc_base/location.h"
#include "ns3/log.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE("TPace");
TPace::~TPace(){
	if(pm_!=NULL){
		pm_->Stop();
		if(send_bucket_!=NULL){
			pm_->DeRegisterModule(send_bucket_.get());
		}
	}
	if(!buf_.empty()){
		sim_segment_t *seg=NULL;
		auto it=buf_.begin();
		seg=it->second;
		buf_.erase(it);
		delete seg;
	}
}
void TPace::UpdateRate(uint32_t bps){
	rate_=bps;
	if(send_bucket_!=NULL){
		send_bucket_->SetEstimatedBitrate(rate_);
	}
}
bool TPace::TimeToSendPacket(uint32_t ssrc,
	                              uint16_t sequence_number,
	                              int64_t capture_time_ms,
	                              bool retransmission,
	                              const webrtc::PacedPacketInfo& cluster_info){
	auto it=buf_.find(sequence_number);
	sim_segment_t *seg=NULL;
	if(it!=buf_.end()){
		seg=it->second;
		buf_.erase(it);
	}
	if(seg){
		uint32_t now=Simulator::Now().GetMilliSeconds();
		seg->send_ts=now-first_ts_-seg->timestamp;
		if(observer_){
			observer_->OnSent(seg->data_size+SIM_SEGMENT_HEADER_SIZE);
			pending_len_-=seg->data_size+SIM_SEGMENT_HEADER_SIZE;
		}
        NS_LOG_INFO("sent out");
		if(now-last_log_pending_ts_>log_pending_time_){
			if(!pending_len_cb_.IsNull()){
				pending_len_cb_(pending_len_);
			}
			last_log_pending_ts_=now;
		}
		delete seg;
	}
    return true;
}
size_t TPace::TimeToSendPadding(size_t bytes,
	                                 const webrtc::PacedPacketInfo& cluster_info){
	NS_LOG_INFO("padding not implement");
	return bytes;
}
void TPace::OnFrame(uint8_t*data,uint32_t len){
	uint32_t timestamp=0;
	int64_t now=Simulator::Now().GetMilliSeconds();
	if(first_ts_==-1){
		first_ts_=now;
		ConfigurePace();
	}
	timestamp=now-first_ts_;
	uint8_t* pos=NULL;
	uint16_t splits[MAX_SPLIT_NUMBER], total=0;
	assert((len/ SIM_VIDEO_SIZE) < MAX_SPLIT_NUMBER);
	total = FrameSplit(splits, len);
	pos = (uint8_t*)data;
	sim_segment_t *seg=NULL;
	uint32_t i=0;
	for(i=0;i<total;i++){
		seg=new sim_segment_t();
		seg->fid = frame_seed_;
		seg->timestamp = timestamp;
		seg->ftype = 0;
		seg->payload_type =0;
		seg->index = i;
		seg->total = total;
		seg->remb = 1;
		seg->send_ts = 0;
		seg->transport_seq =trans_seq_;
		seg->data_size = splits[i];
		memcpy(seg->data, pos, seg->data_size);
		pos += splits[i];
		buf_.insert(std::make_pair(trans_seq_,seg));
		send_bucket_->InsertPacket(webrtc::PacedSender::kNormalPriority,uid_,
				trans_seq_,now,seg->data_size+SIM_SEGMENT_HEADER_SIZE,false);
		pending_len_+=seg->data_size+SIM_SEGMENT_HEADER_SIZE;
		packet_seed_++;
		trans_seq_++;
	}
	frame_seed_++;
	if(last_log_pending_ts_==0){
		last_log_pending_ts_=now;
	}
}
void TPace::ConfigurePace(){
	pm_.reset(new ProcessModule());
	send_bucket_.reset(new webrtc::PacedSender(&clock_, this, nullptr));
	send_bucket_->SetEstimatedBitrate(rate_);
	send_bucket_->SetProbingEnabled(false);
	pm_->RegisterModule(send_bucket_.get(),RTC_FROM_HERE);
	pm_->Start();
}
}
