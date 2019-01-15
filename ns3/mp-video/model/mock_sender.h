#ifndef NS3_MPVIDEO_MOCK_SENDER_H_
#define NS3_MPVIDEO_MOCK_SENDER_H_
#include <memory>
#include <map>
#include <list>
#include "videosink.h"
#include "video_proto.h"
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/event-id.h"
#include "ns3/callback.h"
#include "net/my_pacing_sender.h"
#include "net/my_controller_interface.h"
#include "ns3/idealpacer.h"
#include "idealcc.h"
#include "ns3/trace_v1.h"
namespace ns3{
class MockSender:public Application,
public quic::BandwidthObserver{
public:
	MockSender(uint32_t min_bps,uint32_t max_bps,int instance,uint8_t cc_ver);
	~MockSender();
	void HeartBeat();
	void OnAck(uint64_t seq);
	void Bind(uint16_t port);
	InetSocketAddress GetLocalAddress();
	void ConfigurePeer(Ipv4Address addr,uint16_t port);
    void OnBandwidthUpdate() override;
	typedef Callback<void,uint32_t>TraceRate;
	void SetRateTraceFunc(TraceRate cb){
		trace_rate_cb_=cb;
	}
	typedef Callback<void,uint32_t,uint32_t>TraceTimeOffset;
	void SetTimeOffsetTraceFunc(TraceTimeOffset cb){
		trace_time_offset_cb_=cb;
	}
	typedef Callback<void,uint32_t,uint32_t>TraceLoss;
	void SetLossTraceFunc(TraceLoss cb){
		trace_loss_cb_=cb;
	}
private:
	virtual void StartApplication() override;
	virtual void StopApplication() override;
	void RecvPacket(Ptr<Socket> socket);
	void SendToNetwork(Ptr<Packet> p);
	void OnIncomingMessage(uint8_t *buf,uint32_t len);
    void SendFakePacket();//for test
	void SendPaddingPacket(quic::QuicTime quic_now);
	void UpdateRtt(uint64_t seq);
	void DetectLoss(uint64_t seq);
	void RemoveSeqDelayMapLe(uint64_t seq);
	uint32_t min_bps_;
	uint32_t max_bps_;
	int64_t bps_{0};
    TraceRate trace_rate_cb_;
    TraceLoss trace_loss_cb_;
    TraceTimeOffset trace_time_offset_cb_;
    Ipv4Address peer_ip_;
    uint16_t peer_port_;
    Ptr<Socket> socket_;
    uint16_t bind_port_;
	uint64_t seq_{1};
	uint64_t recv_seq_{0};
    EventId heart_timer_;
    uint32_t heart_beat_t_{1};//1 ms;
    quic::MyPacingSender pacer_;
    quic::CongestionController *cc_{NULL};
    uint32_t cur_rtt_{0};
    std::map<uint64_t,int32_t> seq_delay_map_;
    TraceState tracer_;
};
}
#endif /* NS3_MPVIDEO_MOCK_SENDER_H_ */
