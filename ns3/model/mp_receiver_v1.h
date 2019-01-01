#ifndef NS3_MPVIDEO_MP_RECEIVER_V1_H_
#define NS3_MPVIDEO_MP_RECEIVER_V1_H_
#include "ns3/interface_v1.h"
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/path_receiver_v1.h"
#include <map>
namespace zsy{
class MpReceiverV1:public MpReceiverInterfaceV1{
public:
	void HeartBeat();
	void RegisteReceiver(ns3::Ptr<ns3::PathReceiverV1> path);
	void OnVideoPacket(std::shared_ptr<VideoPacketReceiveWrapper> packet) override;
/*	typedef ns3::Callback<void,uint32_t,uint8_t,uint8_t,uint32_t>TraceRecvFrame;
	void SetRecvFrameTraceFunc(TraceRecvFrame cb){
		trace_recv_frame_cb_=cb;
	}*/
	//fid,delay,recv,total;
	typedef ns3::Callback<void,uint32_t,uint32_t,uint8_t,uint8_t>TraceFrameDelay;
	void SetFrameDelayTraceFunc(TraceFrameDelay cb){
		trace_frame_delay_cb_=cb;
	}
	void StopReceiver() override;
	void CheckDeliverable(uint32_t now);
	void DeliverFrame(uint32_t fid,std::shared_ptr<VideoFrameBuffer> frame,bool is_completed);
private:
	bool first_packet_{true};
	bool runing_{true};
    //TraceRecvFrame trace_recv_frame_cb_;
    TraceFrameDelay trace_frame_delay_cb_;
    std::map<uint8_t,ns3::Ptr<ns3::PathReceiverV1>> path_receivers_;
    std::map<uint32_t,std::shared_ptr<VideoFrameBuffer>> frames_;
    uint32_t delivered_fid_{0};
    ns3::EventId heart_timer_;
    uint32_t heart_beat_t_{1};//1 ms;
};
}



#endif /* NS3_MPVIDEO_MP_RECEIVER_V1_H_ */
