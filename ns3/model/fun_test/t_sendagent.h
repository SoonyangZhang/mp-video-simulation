#ifndef NS3_MPVIDEO_FUN_TEST_T_SENDAGENT_H_
#define NS3_MPVIDEO_FUN_TEST_T_SENDAGENT_H_
#include <memory>
#include <vector>
#include "t_interface.h"// introduce mpcommon to supress error from webrtc
#include "modules/pacing/paced_sender.h"
#include "system_wrappers/include/clock.h"
#include "modules/rtp_rtcp/source/rtcp_packet/transport_feedback.h"
#include "modules/congestion_controller/transport_feedback_adapter.h"

#include "ns3/simulator.h"
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/event-id.h"
#include "ns3/processmodule.h"
#include "ns3/proxyobserver.h"
#include "ns3/mpvideoheader.h"
#include "ns3/simulationclock.h"
#include "ns3/callback.h"
#include "ns3/sim_proto.h"
#include "t_pacer_sender.h"
namespace ns3{
class TSendAgent :public DataTarget,
public webrtc::PacedSender::PacketSender,
public Application{
public:
	TSendAgent();
	~TSendAgent();
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

	void Bind(uint16_t port);
	InetSocketAddress GetLocalAddress();
	void SetDestination(InetSocketAddress &addr);
private:
	virtual void StartApplication() override;
	virtual void StopApplication() override;
	void RecvPacket(Ptr<Socket> socket);
	void SendToNetwork(uint8_t*data,uint32_t len);
	void ProcessingMsg(bin_stream_t *stream);
	void IncomingFeedBack(sim_feedback_t* feedback);
	std::vector<webrtc::PacketFeedback> ReceivedPacketFeedbackVector(
			const std::vector<webrtc::PacketFeedback>& input);
    void SortPacketFeedbackVector(
            std::vector<webrtc::PacketFeedback>* const input);
	bool put(sim_segment_t*seg);
	void ConfigurePace();

    Ipv4Address peer_ip_;
    uint16_t peer_port_;
    Ptr<Socket> socket_;
    uint16_t bind_port_;
    bin_stream_t	stream_;
    bool running_{false};
	uint32_t frame_seed_{1};
	uint16_t trans_seq_{1};
	uint32_t packet_seed_{1};
	std::unique_ptr<ProcessModule> pm_;
	SimulationClock clock_;
	//std::unique_ptr<webrtc::PacedSender> send_bucket_;
    std::unique_ptr<TPacedSender> send_bucket_;
	std::map<uint16_t,sim_segment_t*> buf_;
	uint32_t pending_len_{0};
	int64_t first_ts_{-1};
	PacketSentObserver *observer_{NULL};
	uint32_t rate_{1000000};
	PendingLatency pending_delay_cb_;
	PendingLength pending_len_cb_;
	uint32_t uid_{1234};
	uint8_t pid_{1};
	uint32_t log_pending_time_{200};
	uint32_t last_log_pending_ts_{0};
	webrtc::TransportFeedbackAdapter transport_feedback_adapter_;
};
}




#endif /* NS3_MPVIDEO_FUN_TEST_T_SENDAGENT_H_ */
