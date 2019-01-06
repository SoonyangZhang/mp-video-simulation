#include "mp_receiver_v1.h"
#include "ns3/simulator.h"
using namespace ns3;
const uint32_t max_frame_wait=300;
namespace zsy{
void MpReceiverV1::HeartBeat(){
	if(heart_timer_.IsExpired()&&runing_){
		Time next=MilliSeconds(heart_beat_t_);
		uint32_t now=Simulator::Now().GetMilliSeconds();
		CheckDeliverable(now);
		heart_timer_=Simulator::Schedule(next,
				&MpReceiverV1::HeartBeat,this);
	}
}
void MpReceiverV1::RegisteReceiver(ns3::Ptr<ns3::PathReceiverV1> path){
	uint8_t pid=path->get_path_id();
	path->RegisterMpSink(this);
	path_receivers_.insert(std::make_pair(pid,path));
}
void MpReceiverV1::OnVideoPacket(std::shared_ptr<VideoPacketReceiveWrapper> packet){
	if(!runing_){
		return;
	}
	if(first_packet_){
		heart_timer_=Simulator::ScheduleNow(&MpReceiverV1::HeartBeat,this);
		first_packet_=false;
	}
	uint32_t fid=packet->segment_->fid;
	uint8_t ftype=packet->segment_->ftype;
	//uint8_t index=packet->segment_->index;
	uint8_t total=packet->segment_->total;
	//uint32_t owd=packet->get_owd();
	if(fid<=delivered_fid_){
		return;
	}
    uint32_t now=Simulator::Now().GetMilliSeconds();
	auto it=frames_.find(fid);
	if(it==frames_.end()){
		std::shared_ptr<VideoFrameBuffer> frame(new VideoFrameBuffer(ftype,
				fid,total));
		frame->OnNewPacket(packet,now);
		frames_.insert(std::make_pair(fid,frame));
	}else{
		std::shared_ptr<VideoFrameBuffer> frame=it->second;
		frame->OnNewPacket(packet,now);
	}
	//if(!trace_recv_frame_cb_.IsNull()){
	//	trace_recv_frame_cb_(fid,index,total,owd);
	//}
}
void MpReceiverV1::StopReceiver(){
	runing_=false;
	if(!heart_timer_.IsExpired()){
		heart_timer_.Cancel();
	}
}
void MpReceiverV1::CheckDeliverable(uint32_t now){
	if(!runing_){
		return;
	}
	while(!frames_.empty()){
		auto it=frames_.begin();
		uint32_t fid=it->first;
		std::shared_ptr<VideoFrameBuffer> frame=it->second;
		if(frame->is_frame_completed()){
			DeliverFrame(fid,frame,true);
			delivered_fid_=fid;
			frames_.erase(it);
		}else if(frame->is_expired(now,max_frame_wait)){
			DeliverFrame(fid,frame,false);
			delivered_fid_=fid;
			frames_.erase(it);
		}else{
			break;
		}
	}
}
void MpReceiverV1::DeliverFrame(uint32_t fid,std::shared_ptr<VideoFrameBuffer>frame,
		bool is_completed){
	uint32_t delay=frame->get_frame_delay();
	uint8_t received=frame->Received();
	uint8_t total=frame->Total();
	if(!trace_frame_delay_cb_.IsNull()){
		trace_frame_delay_cb_(fid,delay,received,total);
	}
}
}




