#include "mp_sender_v1.h"
#include "mpcommon.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <assert.h>
using namespace ns3;
namespace zsy{
NS_LOG_COMPONENT_DEFINE("MpSenderV1");
const uint32_t max_packet_delay_threshold=500;//milliseconds
MpSenderV1::MpSenderV1(){

}
MpSenderV1::~MpSenderV1(){
	if(!heart_timer_.IsExpired()){
		heart_timer_.Cancel();
	}
}
void MpSenderV1::SetRun(){
	if(timer_trigger_){
		return;
	}
	timer_trigger_=true;
	heart_timer_=Simulator::ScheduleNow(&MpSenderV1::Process,this);
}
void MpSenderV1::Process(){
	uint32_t now=Simulator::Now().GetMilliSeconds();
	if(!runing_){
		return;
	}
	int path_available=usable_paths_.size();
	if(heart_timer_.IsExpired()&&runing_){
		if(path_available>=1){
			source_->HeartBeat(now);
			if(now>next_stale_check_){
				CheckStalePacket(now);
				next_stale_check_=now+stale_check_gap_;
			}
		}
		Time next=MilliSeconds(heart_beat_t_);
		heart_timer_=Simulator::Schedule(next,&MpSenderV1::Process,this);
	}
}
void MpSenderV1::RegisterSender(ns3::Ptr<ns3::PathSenderV1> sender){
	uint8_t pid=0;
	pid=sender->get_path_id();
	if(pid==0){
		pid=path_id_allocator_;
		path_id_allocator_++;
		sender->set_path_id(pid);
	}
	auto it=waitting_paths_.find(pid);
	if(it==waitting_paths_.end()){
		waitting_paths_.insert(std::make_pair(pid,sender));
		sender->ConfigureFps(fps_);
		sender->RegisterMpsender(this);
	}
}
void MpSenderV1::OnRateIncrease(uint8_t pid,uint32_t bps){

}
void MpSenderV1::OnRateDecrease(uint8_t pid,uint32_t bps){

}
void MpSenderV1::OnRateUpdate(){
	uint32_t bps=0;
	for(auto it=usable_paths_.begin();
			it!=usable_paths_.end();it++){
		Ptr<PathSenderV1> path=it->second;
		bps+=path->get_rate();
	}
    //NS_LOG_INFO("add "<<bps);
	source_->ChangeRate(bps);
	if(!trace_rate_cb_.IsNull()){
		trace_rate_cb_(bps);
	}
}
void MpSenderV1::OnVideoFrame(int ftype,void *data,
		uint32_t size){
	int64_t now=Simulator::Now().GetMilliSeconds();
	if(first_ts_==0){
		first_ts_=now;
	}
    //NS_LOG_INFO("Frame len "<<size);
	uint32_t timestamp=0;
	timestamp=now-first_ts_;
	uint8_t* pos;
	uint32_t i=0;
	uint16_t splits[MAX_SPLIT_NUMBER], total=0;
	assert((size / SIM_VIDEO_SIZE) < MAX_SPLIT_NUMBER);
	total = FrameSplit(splits, size);
	assert(total<256);
	pos = (uint8_t*)data;
	video_segment_t seg;
	seg.total=total;
	std::map<uint32_t,uint32_t>id_len_map;
	for(i=0;i<total;i++){
		seg.fid=fid_;
		seg.timestamp=timestamp;
		seg.index=i;
		seg.ftype=ftype;
		seg.data_size=splits[i];
		memcpy(seg.data,pos,seg.data_size);
		pos+=seg.data_size;
		std::shared_ptr<VideoPacketWrapper> packet(
				new VideoPacketWrapper(packet_id_,first_ts_));
		packet->OnNewSegment(&seg,now);
		pend_buf_.insert(std::make_pair(packet_id_,packet));
		id_len_map.insert(std::make_pair(packet_id_,seg.data_size));
		packet_id_++;
	}
    fid_++;
	if(schedule_){
		schedule_->IncomingPackets(id_len_map,size);
	}
}
void MpSenderV1::OnNetworkAvailable(uint8_t pid){
	auto it=waitting_paths_.find(pid);
	if(it!=waitting_paths_.end()){
		Ptr<PathSenderV1> sender=it->second;
		waitting_paths_.erase(it);
		usable_paths_.insert(std::make_pair(pid,sender));
	}
	schedule_->RegisterPath(pid);
	SetRun();
}
void MpSenderV1::OnNetworkStop(uint8_t pid){
	schedule_->UnregisterPath(pid);
	runing_=false;
}
void MpSenderV1::OnPacketLost(uint8_t pid,uint32_t packet_id){
	schedule_->RetransPackets(packet_id,pid);
}
void MpSenderV1::CheckStalePacket(uint32_t now){
	while(!sent_buf_.empty()){
		auto it=sent_buf_.begin();
		uint32_t packet_id=it->first;
		std::shared_ptr<VideoPacketWrapper> packet=it->second;
		if(packet->IsStale(now,max_packet_delay_threshold)){
			//NS_LOG_INFO("stale "<<packet_id);
			sent_buf_.erase(it);
		}else{
			break;
		}
	}
}
void MpSenderV1::PacketSchedule(uint32_t packet_id,uint8_t pid){
	if(!runing_){ return ;}
	auto packet_it=pend_buf_.find(packet_id);
	std::shared_ptr<VideoPacketWrapper> packet=packet_it->second;
	pend_buf_.erase(packet_it);
	auto path_it=usable_paths_.find(pid);
	if(path_it!=usable_paths_.end()){
		Ptr<PathSenderV1> path=path_it->second;
		path->OnVideoPacket(packet_id,packet,false);
		sent_buf_.insert(std::make_pair(packet_id,packet));
	}else{
		NS_LOG_ERROR("path not found");
	}

}
int64_t MpSenderV1::GetFirstTs(){
	return first_ts_;
}
Ptr<PathSenderV1> MpSenderV1::GetPathInfo(uint8_t pid){
	Ptr<PathSenderV1> path=NULL;
	auto path_it=usable_paths_.find(pid);
	if(path_it!=usable_paths_.end()){
		path=path_it->second;
	}
	return path;
}
void MpSenderV1::PacketRetrans(uint32_t packet_id,uint8_t pid){
	auto packet_it=sent_buf_.find(packet_id);
	if(packet_it!=sent_buf_.end()){
		std::shared_ptr<VideoPacketWrapper> packet=packet_it->second;
		auto path_it=usable_paths_.find(pid);
		if(path_it!=usable_paths_.end()){
			Ptr<PathSenderV1> path=path_it->second;
			path->OnVideoPacket(packet_id,packet,true);
		}else{
			NS_LOG_ERROR("path not found");
		}
	}
}
}




