#ifndef NS3_MPVIDEO_CONGESTION_BBR_SENDER_V2_H_
#define NS3_MPVIDEO_CONGESTION_BBR_SENDER_V2_H_
#include "net/third_party/quic/core/congestion_control/windowed_filter.h"
#include "net/my_quic_random.h"
#include "net/my_controller_interface.h"
#include <map>
#include <vector>
#include <list>
#include "net/third_party/quic/core/congestion_control/windowed_filter.h"
#include "net/my_quic_random.h"
#include "net/my_controller_interface.h"
#include <map>
#include <vector>
#include <list>
#include <memory>
#include <string>
#include "ns3/callback.h"
namespace quic{
class MySampler{
public:
	MySampler();
	~MySampler();
	  void OnPacketSent(QuicTime sent_time,
	                    QuicPacketNumber packet_number,
	                    QuicByteCount bytes,
	                    QuicByteCount bytes_in_flight);
	  QuicBandwidth OnPacketAcknowledged(QuicTime ack_time,
	                                       QuicPacketNumber packet_number,
										   bool round_update);
	  void OnPacketLost(QuicPacketNumber packet_number);
private:
	  // The total number of congestion controlled bytes sent during the connection.
	  QuicByteCount total_bytes_sent_;

	  // The total number of congestion controlled bytes which were acknowledged.
	  QuicByteCount total_bytes_acked_;

	  // The value of |total_bytes_sent_| at the time the last acknowledged packet
	  // was sent. Valid only when |last_acked_packet_sent_time_| is valid.
	  QuicByteCount total_bytes_sent_at_last_acked_packet_;

	  // The time at which the last acknowledged packet was sent. Set to
	  // QuicTime::Zero() if no valid timestamp is available.
	  QuicTime last_acked_packet_sent_time_;

	  // The time at which the most recent packet was acknowledged.
	  QuicTime last_acked_packet_ack_time_;

	  // The most recently sent packet.
	  QuicPacketNumber last_sent_packet_;
private:
	  struct PacketState{
		  // Time at which the packet is sent.
		    QuicTime sent_time;

		    // Size of the packet.
		    QuicByteCount size;

		    // The value of |total_bytes_sent_| at the time the packet was sent.
		    // Includes the packet itself.
		    QuicByteCount total_bytes_sent;

		    // The value of |total_bytes_sent_at_last_acked_packet_| at the time the
		    // packet was sent.
		    QuicByteCount total_bytes_sent_at_last_acked_packet;

		    // The value of |last_acked_packet_sent_time_| at the time the packet was
		    // sent.
		    QuicTime last_acked_packet_sent_time;

		    // The value of |last_acked_packet_ack_time_| at the time the packet was
		    // sent.
		    QuicTime last_acked_packet_ack_time;

		    // The value of |total_bytes_acked_| at the time the packet was
		    // sent.
		    QuicByteCount total_bytes_acked_at_the_last_acked_packet;
		    // Snapshot constructor. Records the current state of the bandwidth
		    // sampler.
		    PacketState(QuicTime sent_time,
		                                QuicByteCount size,
		                                const MySampler& sampler)
		        : sent_time(sent_time),
		          size(size),
		          total_bytes_sent(sampler.total_bytes_sent_),
		          total_bytes_sent_at_last_acked_packet(
		              sampler.total_bytes_sent_at_last_acked_packet_),
		          last_acked_packet_sent_time(sampler.last_acked_packet_sent_time_),
		          last_acked_packet_ack_time(sampler.last_acked_packet_ack_time_),
		          total_bytes_acked_at_the_last_acked_packet(
		              sampler.total_bytes_acked_){}

		    // Default constructor.  Required to put this structure into
		    // PacketNumberIndexedQueue.
		    PacketState()
		        : sent_time(QuicTime::Zero()),
		          size(0),
		          total_bytes_sent(0),
		          total_bytes_sent_at_last_acked_packet(0),
		          last_acked_packet_sent_time(QuicTime::Zero()),
		          last_acked_packet_ack_time(QuicTime::Zero()),
		          total_bytes_acked_at_the_last_acked_packet(0){}
	  };
	  std::shared_ptr<PacketState> GetPacketInfoEntry(QuicPacketNumber seq);
	  void PacketInfoRemove(QuicPacketNumber seq);
	  QuicBandwidth OnPacketAcknowledgedInner(
	      QuicTime ack_time,
	      QuicPacketNumber packet_number,
	      const PacketState& sent_packet,bool round_update);
	  std::map<QuicPacketNumber,std::shared_ptr<PacketState>> packet_infos_;
};
class MyBbrSenderV2:public BandwidthEstimateInteface,
public CongestionController{
public:
	enum Mode{
			ST_START,
			ST_DRAIN,
			ST_INCREASE,
			ST_DECREASE,
		};
	MyBbrSenderV2(uint64_t min_bps,BandwidthObserver *observer);
	~MyBbrSenderV2();
	QuicBandwidth PaceRate() override;
	QuicBandwidth GetReferenceRate() override;
	QuicBandwidth BandwidthEstimate();
	bool ShouldSendProbePacket() override;
	void OnAck(QuicTime event_time,
			QuicPacketNumber packet_number) override;
	void OnPacketSent(QuicTime event_time,
			QuicPacketNumber packet_number,
			QuicByteCount bytes) override;
	void OnEstimateBandwidth(SentClusterInfo *cluster,uint64_t cluster_id,QuicBandwidth sent_bw,
QuicBandwidth acked_bw,bool is_probe) override{

	}
	typedef ns3::Callback<void,uint64_t,std::string> TraceState;
	void SetStateTraceFunc(TraceState cb){
		trace_state_cb_=cb;
	}
private:
	TraceState trace_state_cb_;
private:
	struct PerPacket{
		QuicTime sent_ts;
		QuicByteCount bytes;
		PerPacket(QuicTime now,QuicByteCount payload)
		:sent_ts(now),bytes(payload){}
	};
	typedef WindowedFilter<QuicBandwidth,
	                       MaxFilter<QuicBandwidth>,
	                       uint64_t,
	                       uint64_t>
	      MaxBandwidthFilter;
	void EnterStartUpMode();
	void MaybeExitStartupOrDrain(QuicTime now);
	void EnterIncreaseMode(QuicTime now);
	// ST_INCREASE-->ST_DECREASE
	// the network is in congestion status.
	void MaybeEnterOrExitDecrease(QuicTime now,
	                              bool min_rtt_expired,bool is_congested,int64_t round);
	void CheckIfFullBandwidthReached();
	bool CheckIfCongestion(QuicBandwidth instant_bw,int64_t round);
	QuicTime::Delta GetMinRtt();
	QuicByteCount GetTargetCongestionWindow(float gain);
	QuicByteCount GetTargetInflightInDecrease(float gain) ;
	void CalculatePacingRate();
	bool UpdateRoundTripCounter(QuicPacketNumber last_acked_packet);
	void AddPacketInfo(QuicTime now,QuicPacketNumber packet_number,QuicByteCount payload);
	void UpdateRttAndInflight(QuicTime now,
			QuicPacketNumber packet_number);
	void UpdateMaxBw();
	std::string GetStateString();
	void PrintDebugInfo(uint64_t bps,std::string state);
	QuicBandwidth min_bw_;
	BandwidthObserver *observer_{NULL};
	Mode mode_{ST_START};
	// The packet number of the most recently sent packet.
	QuicPacketNumber last_sent_packet_{0};
	QuicPacketNumber last_ack_seq_{0};
	// Acknowledgement of any packet after |current_round_trip_end_| will cause
	// the round trip counter to advance.
	QuicPacketCount current_round_trip_end_{0};
	uint64_t round_trip_count_{0};
	std::map<QuicPacketNumber,std::shared_ptr<PerPacket>> sent_packets_map_;
	QuicTime::Delta min_rtt_;
	QuicTime min_rtt_timestamp_;
	QuicTime::Delta min_rtt_in_decrease_;
	QuicTime min_rtt_timestamp_in_decrease_;
	QuicTime::Delta min_rtt_record_;
	QuicTime exit_probe_rtt_at_;
	QuicBandwidth max_bw_in_increase_;
	int64_t congestion_start_round_{0};
	int64_t congestion_round_count_{0};
	QuicBandwidth pacing_rate_;
    QuicBandwidth sending_rate_{QuicBandwidth::Zero()};
	MaxBandwidthFilter max_bandwidth_;
	float pacing_gain_{1.0};
	// The pacing gain applied during the STARTUP phase.
	float high_gain_;
	  // The pacing gain applied during the DRAIN phase.
	float drain_gain_;
	// The number of RTTs to stay in STARTUP mode.  Defaults to 3.
	int64_t num_startup_rtts_;
	//max_bw_in_increase_*loss_gain, last 3 times, the link is in congestion status
	int64_t num_bw_decrease_rtts_;
	// Indicates whether the connection has reached the full bandwidth mode.
	bool is_at_full_bandwidth_{false};
	// Number of rounds during which there was no significant bandwidth increase.
	uint64_t rounds_without_bandwidth_gain_{0};
	// The bandwidth compared to which the increase is measured.
	QuicBandwidth bandwidth_at_last_round_;
	QuicByteCount bytes_in_flight_{0};
	// The initial value of the |congestion_window_|.
	QuicByteCount initial_congestion_window_;
    float dynamic_congestion_back_off_{0.0};
	MySampler sampler_;
};
}
#endif /* NS3_MPVIDEO_CONGESTION_BBR_SENDER_V2_H_ */
