#ifndef NS3_MPVIDEO_IDEALENCODER_H_
#define NS3_MPVIDEO_IDEALENCODER_H_
#include "videosink.h"
#include <stdio.h>
enum Frame_Type{
key,
not_key,
};
namespace zsy{
class IdealEncoder:public VideoSource{
public:
	IdealEncoder(uint32_t min_bps,uint32_t start_bps,
			uint32_t max_bps);
	~IdealEncoder(){}
	void set_fps(uint32_t fp){fps_=fp;}
	void HeartBeat(uint32_t now) override;
	void RegisterSink(VideoSink *sink) override{
		sink_=sink;
	}
	void ChangeRate(uint32_t bps) override;
private:
	uint32_t min_bps_;
	uint32_t rate_;
	uint32_t max_bps_;
	uint32_t fps_{25};
	VideoSink *sink_{NULL};
	uint32_t next_{0};
	uint32_t fid_{1};
};
}



#endif /* NS3_MPVIDEO_IDEALENCODER_H_ */
