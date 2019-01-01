#include "idealencoder.h"
namespace zsy{
IdealEncoder::IdealEncoder(uint32_t min_bps,uint32_t start_bps,
		uint32_t max_bps)
:min_bps_(min_bps)
,rate_(start_bps)
,max_bps_(max_bps){
}
void IdealEncoder::HeartBeat(uint32_t now){
	if(now>=next_){
		uint8_t frame_type=Frame_Type::not_key;
		if(fid_%fps_==1){
			frame_type=Frame_Type::key;
		}
		uint32_t len=rate_/(fps_*8);
		next_=now+1000/fps_;
		char *data=new char [len];
		if(sink_){
			sink_->OnVideoFrame(frame_type,data,len);
		}
		fid_++;
		delete data;
}
}
void IdealEncoder::ChangeRate(uint32_t bps){
	uint32_t target=bps;
	if(target<min_bps_){
		target=min_bps_;
	}
	if(target>max_bps_){
		target=max_bps_;
	}
	rate_=target;
}
}
