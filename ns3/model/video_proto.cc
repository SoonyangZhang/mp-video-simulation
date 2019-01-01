#include "video_proto.h"
#include <memory.h>
#include "net/third_party/quic/platform/api/quic_endian.h"
#include "net/third_party/quic/core/quic_data_reader.h"
#include "net/third_party/quic/core/quic_data_writer.h"
namespace zsy{
VideoPacketWrapper::VideoPacketWrapper(uint32_t packet_id
		,uint32_t ref_time){
	packet_id_=packet_id;
	ref_time_=ref_time;
}
VideoPacketWrapper::~VideoPacketWrapper(){
	if(segment_){
		delete segment_;
	}
}
void VideoPacketWrapper::OnNewSegment(video_segment_t *seg,uint32_t now){
	if(segment_){
		return ;
	}
	segment_=new video_segment_t();
	memcpy(segment_,seg,sizeof(video_segment_t));
	timestamp_=segment_->timestamp;
	generate_ts_=now;
	ftype_=segment_->ftype;
	payload_size_=VIDEO_SEGMENT_HEADER_SIZE+segment_->data_size;
}
bool VideoPacketWrapper::IsStale(uint32_t now,uint32_t threshold){
	bool stale=false;
	uint32_t reative_ts=now-generate_ts_;
	if(reative_ts>threshold){
		stale=true;
	}
	return stale;
}
int VideoPacketWrapper::Serialize(uint8_t *buf,uint32_t len,uint32_t now){
	send_ts_=now-ref_time_-timestamp_;
	int ret=0;
	if(!segment_){
		ret=-1;
	}
	if(segment_){
		segment_->send_ts=(uint16_t)send_ts_;
		if(payload_size_>len){
			ret=-1;
		}else{
			quic::QuicDataWriter writer(len, (char*)buf, quic::NETWORK_BYTE_ORDER);
			writer.WriteUInt32(segment_->fid);
			writer.WriteUInt32(segment_->timestamp);
			writer.WriteUInt8(segment_->index);
			writer.WriteUInt8(segment_->total);
			writer.WriteUInt8(segment_->ftype);
			writer.WriteUInt16(segment_->send_ts);
			writer.WriteUInt16(segment_->data_size);
			writer.WriteBytes(segment_->data,segment_->data_size);
			ret=payload_size_;
		}
	}
	return ret;
}
VideoPacketReceiveWrapper::VideoPacketReceiveWrapper(uint32_t ts)
:received_ts_(ts){}
VideoPacketReceiveWrapper::~VideoPacketReceiveWrapper(){
	if(segment_){
		delete segment_;
	}
}
void VideoPacketReceiveWrapper::Deserialize(uint8_t*buf,uint32_t len){
	if(!segment_){
		segment_=new video_segment_t();
	}
	quic::QuicDataReader reader((char*)buf,len,quic::NETWORK_BYTE_ORDER);
	reader.ReadUInt32(&segment_->fid);
	reader.ReadUInt32(&segment_->timestamp);
	reader.ReadUInt8(&segment_->index);
	reader.ReadUInt8(&segment_->total);
	reader.ReadUInt8(&segment_->ftype);
	reader.ReadUInt16(&segment_->send_ts);
	reader.ReadUInt16(&segment_->data_size);
	reader.ReadBytes(segment_->data,segment_->data_size);
}

uint32_t VideoPacketReceiveWrapper::get_owd(){
	uint32_t owd=0;
	sent_ts_=segment_->timestamp+segment_->send_ts+first_ts_;
	owd=received_ts_-sent_ts_;
	return owd;
}
uint32_t VideoPacketReceiveWrapper::get_frame_timestamp(){
	uint32_t frame_timestamp=segment_->timestamp;//+first_ts_;
	return frame_timestamp;
}
VideoFrameBuffer::VideoFrameBuffer(uint8_t ftype,
		uint32_t fid,uint8_t total)
:ftype_(ftype)
,fid_(fid)
,total_(total){

}
bool VideoFrameBuffer::is_expired(uint32_t now,uint32_t threshold){
	bool expired=false;
	if(now-max_recv_ts_>threshold){
		expired=true;
	}
	return expired;
}
void VideoFrameBuffer::OnNewPacket(std::shared_ptr<VideoPacketReceiveWrapper> packet,
		uint32_t ts){
	int16_t index=packet->get_index();
	if(index==-1){
		return;
	}
	auto it=packets_.find(index);
	if(it!=packets_.end()){
		return ;
	}
	max_recv_ts_=ts;
	if(frame_timestamp_==0){
		frame_timestamp_=packet->get_frame_timestamp();
	}
	packets_.insert(std::make_pair(index,packet));
	received_++;
	if(received_==total_){
		is_completed_=true;
	}
}
}
