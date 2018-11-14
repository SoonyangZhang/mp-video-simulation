#ifndef NS3_MPVIDEO_FUN_TEST_T_RECVAGENT_H_
#define NS3_MPVIDEO_FUN_TEST_T_RECVAGENT_H_
#include <memory>
#include "ns3/mpcommon.h"
#include "ns3/sim_proto.h"
#include "ns3/cf_stream.h"
#include "rtc_base/timeutils.h"
#include "modules/remote_bitrate_estimator/remote_estimator_proxy.h"
#include "modules/rtp_rtcp/source/rtcp_packet/transport_feedback.h"
#include "modules/pacing/packet_router.h"

#include "ns3/simulator.h"
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/event-id.h"
#include "ns3/processmodule.h"
#include "ns3/mpvideoheader.h"
#include "ns3/simulationclock.h"
#include "ns3/callback.h"
namespace ns3{
class TRecvAgent:public Application,
public webrtc::TransportFeedbackSenderInterface{
public:
    TRecvAgent();
    ~TRecvAgent();
	void Bind(uint16_t port);
	InetSocketAddress GetLocalAddress();
	bool SendTransportFeedback(webrtc::rtcp::TransportFeedback* packet) override;
private:
	virtual void StartApplication() override;
	virtual void StopApplication() override;
	void RecvPacket(Ptr<Socket> socket);
	void ProcessingMsg(bin_stream_t *stream);
	void ConfigureRemoteProxy();
	void OnReceiveSegment(sim_header_t *header,sim_segment_t *seg);
    void SendToNetwork(uint8_t*data,uint32_t len);
    Ipv4Address peer_ip_;
    uint16_t peer_port_;
    Ptr<Socket> socket_;
    uint16_t bind_port_;
    uint32_t uid_{4321};
	uint8_t pid_;
	bool first_packet_{true};
	SimulationClock m_clock;
	bin_stream_t	stream_;
	std::unique_ptr<ProcessModule> pm_;
	bool running_{false};
	std::unique_ptr<webrtc::RemoteEstimatorProxy> remote_estimator_proxy_;
};
}



#endif /* NS3_MPVIDEO_FUN_TEST_T_RECVAGENT_H_ */
