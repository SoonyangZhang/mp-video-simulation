/*
 * copy from webrtc m65
 */
#ifndef NS3_MPVIDEO_FUN_TEST_T_PACER_SENDER_H_
#define NS3_MPVIDEO_FUN_TEST_T_PACER_SENDER_H_
#include <memory>
#include "ns3/mpcommon.h"
#include "api/optional.h"
#include "modules/pacing/pacer.h"
#include "modules/pacing/packet_queue2.h"
#include "rtc_base/criticalsection.h"
#include "rtc_base/thread_annotations.h"
#include "typedefs.h"  // NOLINT(build/include)

#include "modules/pacing/paced_sender.h"
#include "modules/pacing/alr_detector.h"
#include "modules/pacing/bitrate_prober.h"
#include "system_wrappers/include/clock.h"
#include "logging/rtc_event_log/rtc_event_log.h"
#include "modules/pacing/interval_budget.h"
namespace ns3{
//class webrtc::ProbeClusterCreatedObserver;


class TPacedSender : public webrtc::Pacer {
 public:
  // Expected max pacer delay in ms. If ExpectedQueueTimeMs() is higher than
  // this value, the packet producers should wait (eg drop frames rather than
  // encoding them). Bitrate sent may temporarily exceed target set by
  // UpdateBitrate() so that this limit will be upheld.
  static const int64_t kMaxQueueLengthMs;
  // Pacing-rate relative to our target send rate.
  // Multiplicative factor that is applied to the target bitrate to calculate
  // the number of bytes that can be transmitted per interval.
  // Increasing this factor will result in lower delays in cases of bitrate
  // overshoots from the encoder.
  static const float kDefaultPaceMultiplier;

  TPacedSender(const webrtc::Clock* clock,
		  webrtc::PacedSender::PacketSender* packet_sender,
		  webrtc::RtcEventLog* event_log);

  TPacedSender(const webrtc::Clock* clock,
		  webrtc::PacedSender::PacketSender* packet_sender,
		  webrtc::RtcEventLog* event_log,
              std::unique_ptr<webrtc::PacketQueue> packets);

  ~TPacedSender() override;

  virtual void CreateProbeCluster(int bitrate_bps);

  // Temporarily pause all sending.
  void Pause();

  // Resume sending packets.
  void Resume();

  // Enable bitrate probing. Enabled by default, mostly here to simplify
  // testing. Must be called before any packets are being sent to have an
  // effect.
  void SetProbingEnabled(bool enabled);

  // Sets the estimated capacity of the network. Must be called once before
  // packets can be sent.
  // |bitrate_bps| is our estimate of what we are allowed to send on average.
  // We will pace out bursts of packets at a bitrate of
  // |bitrate_bps| * kDefaultPaceMultiplier.
  void SetEstimatedBitrate(uint32_t bitrate_bps) override;

  // Sets the minimum send bitrate and maximum padding bitrate requested by send
  // streams.
  // |min_send_bitrate_bps| might be higher that the estimated available network
  // bitrate and if so, the pacer will send with |min_send_bitrate_bps|.
  // |max_padding_bitrate_bps| might be higher than the estimate available
  // network bitrate and if so, the pacer will send padding packets to reach
  // the min of the estimated available bitrate and |max_padding_bitrate_bps|.
  void SetSendBitrateLimits(int min_send_bitrate_bps,
                            int max_padding_bitrate_bps);

  // Returns true if we send the packet now, else it will add the packet
  // information to the queue and call TimeToSendPacket when it's time to send.
  void InsertPacket(webrtc::RtpPacketSender::Priority priority,
                    uint32_t ssrc,
                    uint16_t sequence_number,
                    int64_t capture_time_ms,
                    size_t bytes,
                    bool retransmission) override;

  // Currently audio traffic is not accounted by pacer and passed through.
  // With the introduction of audio BWE audio traffic will be accounted for
  // the pacer budget calculation. The audio traffic still will be injected
  // at high priority.
  void SetAccountForAudioPackets(bool account_for_audio) override;

  // Returns the time since the oldest queued packet was enqueued.
  virtual int64_t QueueInMs() const;

  virtual size_t QueueSizePackets() const;

  // Returns the time when the first packet was sent, or -1 if no packet is
  // sent.
  virtual int64_t FirstSentPacketTimeMs() const;

  // Returns the number of milliseconds it will take to send the current
  // packets in the queue, given the current size and bitrate, ignoring prio.
  virtual int64_t ExpectedQueueTimeMs() const;

  // Returns time in milliseconds when the current application-limited region
  // started or empty result if the sender is currently not application-limited.
  //
  // Application Limited Region (ALR) refers to operating in a state where the
  // traffic on network is limited due to application not having enough
  // traffic to meet the current channel capacity.
  virtual rtc::Optional<int64_t> GetApplicationLimitedRegionStartTime() const;

  // Returns the number of milliseconds until the module want a worker thread
  // to call Process.
  int64_t TimeUntilNextProcess() override;

  // Process any pending packets in the queue(s).
  void Process() override;

  // Called when the prober is associated with a process thread.
  void ProcessThreadAttached(webrtc::ProcessThread* process_thread) override;
  void SetPacingFactor(float pacing_factor);
  float GetPacingFactor() const;
  void SetQueueTimeLimit(int limit_ms);

 private:
  // Updates the number of bytes that can be sent for the next time interval.
  void UpdateBudgetWithElapsedTime(int64_t delta_time_in_ms)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(critsect_);
  void UpdateBudgetWithBytesSent(size_t bytes)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(critsect_);

  bool SendPacket(const webrtc::PacketQueue::Packet& packet,
                  const webrtc::PacedPacketInfo& cluster_info)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(critsect_);
  size_t SendPadding(size_t padding_needed, const webrtc::PacedPacketInfo& cluster_info)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(critsect_);

  const webrtc::Clock* const clock_;
  webrtc::PacedSender::PacketSender* const packet_sender_;
  const std::unique_ptr<webrtc::AlrDetector> alr_detector_ RTC_PT_GUARDED_BY(critsect_);

  rtc::CriticalSection critsect_;
  bool paused_ RTC_GUARDED_BY(critsect_);
  // This is the media budget, keeping track of how many bits of media
  // we can pace out during the current interval.
  const std::unique_ptr<webrtc::IntervalBudget> media_budget_
      RTC_PT_GUARDED_BY(critsect_);
  // This is the padding budget, keeping track of how many bits of padding we're
  // allowed to send out during the current interval. This budget will be
  // utilized when there's no media to send.
  const std::unique_ptr<webrtc::IntervalBudget> padding_budget_
      RTC_PT_GUARDED_BY(critsect_);

  const std::unique_ptr<webrtc::BitrateProber> prober_ RTC_PT_GUARDED_BY(critsect_);
  bool probing_send_failure_ RTC_GUARDED_BY(critsect_);
  // Actual configured bitrates (media_budget_ may temporarily be higher in
  // order to meet pace time constraint).
  uint32_t estimated_bitrate_bps_ RTC_GUARDED_BY(critsect_);
  uint32_t min_send_bitrate_kbps_ RTC_GUARDED_BY(critsect_);
  uint32_t max_padding_bitrate_kbps_ RTC_GUARDED_BY(critsect_);
  uint32_t pacing_bitrate_kbps_ RTC_GUARDED_BY(critsect_);

  int64_t time_last_update_us_ RTC_GUARDED_BY(critsect_);
  int64_t first_sent_packet_ms_ RTC_GUARDED_BY(critsect_);

  const std::unique_ptr<webrtc::PacketQueue> packets_ RTC_PT_GUARDED_BY(critsect_);
  uint64_t packet_counter_ RTC_GUARDED_BY(critsect_);
  webrtc::ProcessThread* process_thread_ = nullptr;

  float pacing_factor_ RTC_GUARDED_BY(critsect_);
  int64_t queue_time_limit RTC_GUARDED_BY(critsect_);
  bool account_for_audio_ RTC_GUARDED_BY(critsect_);
};
}




#endif /* NS3_MPVIDEO_FUN_TEST_T_PACER_SENDER_H_ */
