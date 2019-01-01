#ifndef NS3_MPVIDEO_MP_SENDER_V1_H_
#define NS3_MPVIDEO_MP_SENDER_V1_H_
#include "videosink.h"
#include "video_proto.h"
#include <memory>
#include "path_sender_v1.h"
#include "interface_v1.h"
#include "ns3/core-module.h"
#include "ns3/callback.h"
namespace zsy{
class MpSenderV1:public VideoSink,
public PathStatusNotifier,
public MpSenderIntefaceV1{
public:
	MpSenderV1();
	~MpSenderV1();
	void Process();
	void set_encoder(VideoSource *source){
		source_=source;
		source->RegisterSink(this);
	}
	void set_schedule(ScheduleV1* schedule){
		schedule_=schedule;
		schedule_->SetSender(this);
	}
	void RegisterSender(ns3::Ptr<ns3::PathSenderV1> sender);
	void OnRateIncrease(uint8_t pid,uint32_t bps) override;
	void OnRateDecrease(uint8_t pid,uint32_t bps) override;
	void OnRateUpdate() override;
	void OnVideoFrame(int ftype,void *data,
			uint32_t size) override;
	uint32_t get_first_ts() override {return first_ts_;}
	void OnNetworkAvailable(uint8_t pid) override;
	void OnNetworkStop(uint8_t pid) override;
	void OnPacketLost(uint8_t pid,uint32_t packet_id) override;
	void CheckStalePacket(uint32_t now);

	void PacketSchedule(uint32_t packet_id,uint8_t pid) override;
	int64_t GetFirstTs() override;
	ns3::Ptr<ns3::PathSenderV1> GetPathInfo(uint8_t pid) override;
	void PacketRetrans(uint32_t packet_id,uint8_t pid);
	typedef ns3::Callback<void,uint32_t> TraceSumRate;
	void SetSumRateTraceFun(TraceSumRate cb){
		trace_rate_cb_=cb;
	}
private:
	void SetRun();
	VideoSource *source_{NULL};
	ScheduleV1 *schedule_{NULL};
	uint32_t packet_id_{1};
	uint32_t fid_{1};
	int64_t first_ts_{0};
	std::map<uint32_t,std::shared_ptr<VideoPacketWrapper>> pend_buf_;
	std::map<uint32_t,std::shared_ptr<VideoPacketWrapper>> sent_buf_;
	std::map<uint8_t,ns3::Ptr<ns3::PathSenderV1>> waitting_paths_;
	std::map<uint8_t,ns3::Ptr<ns3::PathSenderV1>> usable_paths_;
	uint32_t path_id_allocator_{1};
	bool runing_{true};
	uint32_t next_stale_check_{0};
	uint32_t stale_check_gap_{100};
	ns3::EventId heart_timer_;
	uint32_t heart_beat_t_{1};
	bool timer_trigger_{false};
	TraceSumRate trace_rate_cb_;
};
}




#endif /* NS3_MPVIDEO_MP_SENDER_V1_H_ */
