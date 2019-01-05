#ifndef NS3_MPVIDEO_PATH_SENDER_V1_H_
#define NS3_MPVIDEO_PATH_SENDER_V1_H_
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
namespace ns3{
class PathSenderV1:public Application,
public quic::BandwidthObserver{
public:
	PathSenderV1(uint32_t min_bps,uint32_t max_bps);
    ~PathSenderV1();
	void HeartBeat();
	void set_path_id(uint8_t pid){
		pid_=pid;
	}
	uint8_t get_path_id(){
		return pid_;
	}
	void RegisterMpsender(zsy::PathStatusNotifier *mpsender){
		mpsender_=mpsender;
	}
	void ConfigureFps(uint32_t fps);
	void OnVideoPacket(uint32_t packet_id,
			std::shared_ptr<zsy::VideoPacketWrapper>packet,bool is_retrans);
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
	uint64_t GetIntantRate(){
		return bps_;
	}
	uint64_t GetPendingLen(){
		return pending_bytes_;
	}
	uint64_t get_rate(){ return bps_;}
	uint32_t get_rtt(){
		return cur_rtt_;
	}
	int64_t get_first_ts(){
		int64_t first_ts=-1;
		if(mpsender_){
			first_ts=mpsender_->get_first_ts();
		}
		return first_ts;
	}
private:
	virtual void StartApplication() override;
	virtual void StopApplication() override;
	void ProcessVideoPacket();
	void SendPingMessage();
	void RecvPacket(Ptr<Socket> socket);
	void SendToNetwork(Ptr<Packet> p);
	void OnIncomingMessage(uint8_t *buf,uint32_t len);
    void SendFakePacket();//for test
	void SendPaddingPacket(quic::QuicTime quic_now);
	void SendStreamPacket(quic::QuicTime quic_now,bool is_retrans);
	void RecordRate(uint32_t now);
	void UpdateRate(uint32_t now);
	void UpdateRtt(uint64_t seq);
	void DetectLoss(uint64_t seq);
	void RemoveSeqDelayMapLe(uint64_t seq);
	void RemoveSeqIdMapLe(uint64_t seq);
	void CheckQueueExceed(uint32_t now);
	void SendPacketWithoutCongestion(std::shared_ptr<zsy::VideoPacketWrapper>packet,
			uint32_t ns_now);
	uint32_t min_bps_;
	uint32_t max_bps_;
	bool first_packet_{true};
    Ipv4Address peer_ip_;
    uint16_t peer_port_;
    Ptr<Socket> socket_;
    uint16_t bind_port_;
	zsy::PathStatusNotifier *mpsender_{NULL};
	uint8_t pid_{0};
	std::list<std::shared_ptr<zsy::VideoPacketWrapper>>sending_queue_;
	std::list<std::shared_ptr<zsy::VideoPacketWrapper>>resending_queue_;
	//packet_id,recived ts from upper layer
	std::map<uint32_t,uint32_t> id_delay_map;
	std::map<uint64_t,uint32_t> seq_id_map_;
	std::map<uint64_t,int32_t> seq_delay_map_;
	uint32_t instant_delay_{0};
	uint64_t seq_{1};
	uint64_t recv_seq_{0};
	bool path_available_{false};
    EventId heart_timer_;
    uint32_t heart_beat_t_{1};//1 ms;
    quic::MyPacingSender pacer_;
    //IdealPacingSender pacer_;//for test;
    quic::CongestionController *cc_{NULL};
    //IdealCC cc_;
    TraceRate trace_rate_cb_;
    TraceLoss trace_loss_cb_;
    TraceTimeOffset trace_time_offset_cb_;
    uint32_t rate_out_next_{0};
    uint32_t cur_rtt_{0};
    uint32_t pending_bytes_{0};
    int64_t bps_{0};
    uint32_t rate_update_next_{0};
    uint32_t over_offset_ts_{0};
};
}




#endif /* NS3_MPVIDEO_PATH_SENDER_V1_H_ */
