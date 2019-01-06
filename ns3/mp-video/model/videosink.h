#ifndef NS3_MPVIDEO_VIDEOSINK_H_
#define NS3_MPVIDEO_VIDEOSINK_H_
#include <stdint.h>
namespace zsy{
class VideoSink{
public:
	virtual void OnVideoFrame(int ftype,void *data,
			uint32_t size)=0;
	virtual ~VideoSink(){}
};
class VideoSource{
public:
	virtual uint32_t get_fps()=0;
	virtual void RegisterSink(VideoSink *sink)=0;
	virtual void HeartBeat(uint32_t now)=0;
	virtual void ChangeRate(uint32_t bps)=0;
	virtual ~VideoSource(){}
};
class PathStatusNotifier{
public:
	virtual ~PathStatusNotifier(){};
	virtual uint32_t get_first_ts()=0;
	virtual void OnNetworkAvailable(uint8_t pid)=0;
	virtual void OnNetworkStop(uint8_t pid)=0;
	virtual void OnPacketLost(uint8_t pid,uint32_t packet_id)=0;
	virtual void OnRateUpdate()=0;
	virtual void OnRateIncrease(uint8_t pid,uint32_t bps)=0;
	virtual void OnRateDecrease(uint8_t pid,uint32_t bps)=0;
};
}
#endif /* NS3_MPVIDEO_VIDEOSINK_H_ */
