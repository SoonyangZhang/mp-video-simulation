#ifndef MULTIPATH_PATHSENDER_H_
#define MULTIPATH_PATHSENDER_H_
#include <map>
#include<list>
#include <queue>

#include "mpcommon.h"
#include "pathcommon.h"
#include "sessioninterface.h"
#include "cf_stream.h"
#include "rtc_base/location.h"
#include "rtc_base/socket.h"
#include "system_wrappers/include/field_trial.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/congestion_controller/include/send_side_congestion_controller.h"
#include "modules/rtp_rtcp/source/rtcp_packet/transport_feedback.h"
#include "modules/pacing/paced_sender.h"
#include "system_wrappers/include/clock.h"

#include "ns3/t_pacer_sender.h"
#include "ns3/simulator.h"
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/event-id.h"
#include "ns3/processmodule.h"
//#include "ns3/proxyobserver.h"
#include "ns3/mpvideoheader.h"
#include "ns3/simulationclock.h"
#include "ns3/callback.h"
#include "net/third_party/quic/core/quic_bandwidth.h"
namespace ns3{
class PathSender:public webrtc::PacedSender::PacketSender,
public webrtc::SendSideCongestionController::Observer,
public Application{
public:
	PathSender();
	~PathSender();
	void SetClock(webrtc::Clock *clock);
	void HeartBeat();
	virtual bool TimeToSendPacket(uint32_t ssrc,
	                              uint16_t sequence_number,
	                              int64_t capture_time_ms,
	                              bool retransmission,
	                              const webrtc::PacedPacketInfo& cluster_info) override;
	virtual size_t TimeToSendPadding(size_t bytes,
	                                 const webrtc::PacedPacketInfo& cluster_info) override;
	void OnNetworkChanged(uint32_t bitrate_bps,
	                      uint8_t fraction_loss,  // 0 - 255.
	                      int64_t rtt_ms,
	                      int64_t probing_interval_ms) override;
	void SetOracleRate(uint32_t bps);
	void SetOracleWithOwnPace(uint32_t bps);
	void ConfigureOracleCongestion();
	void ConfigureNoCongestion();
	void ConfigureCongestion();
	uint32_t GetFirstTs();
	bool put(sim_segment_t*seg);
	sim_segment_t *get_segment(uint16_t seq,bool retrans,uint32_t ts);
	sim_segment_t *get_segment();
	uint32_t get_delay();
	uint32_t get_len();
    uint32_t GetCost();
    uint32_t GetSmoothCost();
    uint32_t GetSmoothRate(){return s_rate_;}
    uint32_t GetIntantRate(){return rate_;}
    uint32_t GetMinRtt(){return min_rtt_;}
    uint32_t GetPendingLen(){return pending_len_;}
	uint32_t GetSentBufSize(){return sent_buf_.size();}
	void UpdateMinRtt(uint32_t rtt);
	void UpdateRtt(uint32_t time,uint32_t now);
	send_buf_t * GetSentPacketInfo(uint32_t seq);
	typedef Callback<void,uint32_t,uint32_t> PendingLatency;
	void SetPendingDelayTrace(PendingLatency cb){pending_delay_cb_=cb;}
	void RecvSegAck(sim_segment_ack_t* ack);
	void IncomingFeedBack(sim_feedback_t* feedback);
	void SenderUpdateBase(uint32_t base_id);
	void SendToNetwork(uint8_t*data,uint32_t len);
	void RegisterSenderInterface(SenderInterface*s){ mpsender_=s;}
	void Stop();
	void SendConnect();
	void SendDisconnect(uint32_t now);
	void SendPingMsg(uint32_t now);
	void Bind(uint16_t port);
	InetSocketAddress GetLocalAddress();
	void CheckPing();
    void SetMaxBw(uint32_t maxBw){
        m_maxBw=maxBw;
    }
private:
	void UpdatePaceQueueDelay(uint32_t sent_ts);
	send_buf_t *AllocateSentBuf();
	void FreePendingBuf();
	void FreeSentBuf();
	void RemoveAckedPacket(uint32_t seq);
	void RemoveSentBufUntil(uint32_t seq);
	void SentBufCollection(uint32_t now);
    bool QueueDropper(sim_segment_t*seg);
    void StartPacing();
    void PacingSendSegment();
    void SendSegment(sim_segment_t *seg);
private:
	virtual void StartApplication() override;
	virtual void StopApplication() override;
	void RecvPacket(Ptr<Socket> socket);
	void ProcessingMsg(bin_stream_t *stream);
	int SendPadding(uint16_t payload_len,uint32_t ts);
public:
    Ipv4Address peer_ip_;
    uint16_t peer_port_;
    Ptr<Socket> socket_;
    uint16_t bind_port_;
	uint8_t pid;
	int state;
	uint8_t con_c;
	uint32_t con_ts;
	uint32_t rtt_;
	uint64_t sum_rtt_;
	uint32_t max_rtt_;
	uint64_t rtt_num_;
	uint32_t min_rtt_;
	uint32_t rtt_var_;
	uint32_t rtt_update_ts_;
	uint32_t ping_resend_;
	uint16_t trans_seq_;
	uint32_t packet_seed_;
	uint32_t rate_;
	uint32_t s_rate_;
	uint32_t base_seq_;
	uint32_t s_sent_ts_;
	int8_t pace_mode_{PaceMode::with_congestion};
private:
	CongestionController* controller_;
	SenderInterface *mpsender_;
	uint32_t sender_last_heart_beat_;
	bin_stream_t	strm_;
	bool stop_;
	bool first_packet_;
	std::map<uint16_t,sim_segment_t*> buf_;
	rtc::CriticalSection buf_mutex_;
	std::list<send_buf_t*> sent_buf_;
	rtc::CriticalSection sent_buf_mutex_;
	std::map<uint16_t,sim_segment_t*> retrans_buf_;
	rtc::CriticalSection retrans_mutex_;
	uint32_t last_sentbuf_collect_ts_;
	uint32_t len_;//byte
    uint32_t pending_len_;
	ProcessModule *pm_;
	webrtc::Clock *clock_;
	webrtc::PacedSender *send_bucket_{NULL};
    //TPacedSender *send_bucket_;
	webrtc::RtcEventLogNullImpl null_log_;
    EventId pingTimer_;
    EventId rxTimer_;
    EventId gcTimer_;
    SimulationClock m_clock;
    PendingLatency pending_delay_cb_;
    EventId m_pacing_;
    bool m_pacing_running_{false};
    uint32_t m_maxBw{2000000};
    quic::QuicBandwidth bw_;
};
}





#endif /* MULTIPATH_PATHSENDER_H_ */
