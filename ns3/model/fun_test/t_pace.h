#ifndef NS3_MPVIDEO_FUN_TEST_T_PACE_H_
#define NS3_MPVIDEO_FUN_TEST_T_PACE_H_
#include <memory>
#include "t_interface.h"// introduce mpcommon to supress error from webrtc
#include "modules/pacing/paced_sender.h"
#include "system_wrappers/include/clock.h"
#include "ns3/simulationclock.h"
#include "ns3/processmodule.h"
#include "ns3/callback.h"
#include "ns3/sim_proto.h"
namespace ns3{
class TPace :public DataTarget,
public webrtc::PacedSender::PacketSender{
public:
	TPace(){};
	~TPace();
	void OnFrame(uint8_t*data,uint32_t len) override;
	void UpdateRate(uint32_t bps);
	virtual bool TimeToSendPacket(uint32_t ssrc,
	                              uint16_t sequence_number,
	                              int64_t capture_time_ms,
	                              bool retransmission,
	                              const webrtc::PacedPacketInfo& cluster_info) override;
	virtual size_t TimeToSendPadding(size_t bytes,
	                                 const webrtc::PacedPacketInfo& cluster_info) override;
	void RegisterObserver(PacketSentObserver *o){observer_=o;};
	typedef Callback<void,uint32_t,uint32_t> PendingLatency;
	void SetPendingDelayTrace(PendingLatency cb){pending_delay_cb_=cb;}
	typedef Callback<void,uint32_t> PendingLength;
	void SetPendingLenTrace(PendingLength cb){pending_len_cb_=cb;}
private:
	bool put(sim_segment_t*seg);
	void ConfigurePace();
	uint32_t frame_seed_{1};
	uint16_t trans_seq_{1};
	uint32_t packet_seed_{1};
	std::unique_ptr<ProcessModule> pm_;
	SimulationClock clock_;
	std::unique_ptr<webrtc::PacedSender> send_bucket_;
	std::map<uint16_t,sim_segment_t*> buf_;
	uint32_t pending_len_{0};
	int64_t first_ts_{-1};
	PacketSentObserver *observer_{NULL};
	uint32_t rate_{1000000};
	PendingLatency pending_delay_cb_;
	PendingLength pending_len_cb_;
	uint32_t uid_{1234};
	uint32_t log_pending_time_{200};
	uint32_t last_log_pending_ts_{0};
};
}




#endif /* NS3_MPVIDEO_FUN_TEST_T_PACE_H_ */
