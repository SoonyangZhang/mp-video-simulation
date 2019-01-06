#include <algorithm>
#include "t_sendagent.h"
#include "ns3/log.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE("TSendAgent");
TSendAgent::TSendAgent():transport_feedback_adapter_(&clock_){
	bin_stream_init(&stream_);
}
TSendAgent::~TSendAgent(){
	if(!buf_.empty()){
		sim_segment_t *seg=NULL;
		auto it=buf_.begin();
		seg=it->second;
		buf_.erase(it);
		delete seg;
	}
	bin_stream_destroy(&stream_);
}
void TSendAgent::UpdateRate(uint32_t bps){
	rate_=bps;
	if(send_bucket_!=NULL){
		send_bucket_->SetEstimatedBitrate(rate_);
	}
}
bool TSendAgent::TimeToSendPacket(uint32_t ssrc,
	                              uint16_t sequence_number,
	                              int64_t capture_time_ms,
	                              bool retransmission,
	                              const webrtc::PacedPacketInfo& cluster_info){
	if(!running_){
		return true;
	}
	auto it=buf_.find(sequence_number);
	sim_segment_t *seg=NULL;
	if(it!=buf_.end()){
		seg=it->second;
		buf_.erase(it);
	}
	sim_header_t header;
	if(seg){
		uint32_t now=Simulator::Now().GetMilliSeconds();
		seg->send_ts=now-first_ts_-seg->timestamp;
		if(observer_){
			observer_->OnSent(seg->data_size+SIM_SEGMENT_HEADER_SIZE);
			pending_len_-=seg->data_size+SIM_SEGMENT_HEADER_SIZE;
		}
		INIT_SIM_HEADER(header, SIM_SEG, uid_);
		header.ver=pid_;
		sim_encode_msg(&stream_, &header, seg);
		SendToNetwork(stream_.data,stream_.used);
        if(!owd_cb_.IsNull()){
            sent_ts_map_.insert(std::make_pair(sequence_number,now));
        }
        
        //NS_LOG_INFO("sent out");
		if(now-last_log_pending_ts_>log_pending_time_){
			if(!pending_len_cb_.IsNull()){
				pending_len_cb_(pending_len_);
			}
			last_log_pending_ts_=now;
		}
		uint32_t overhead=seg->data_size+SIM_SEGMENT_HEADER_SIZE;
		transport_feedback_adapter_.AddPacket(uid_, sequence_number, overhead,
				webrtc::PacedPacketInfo());
		transport_feedback_adapter_.OnSentPacket(sequence_number,now);
		delete seg;
	}
    return true;
}
size_t TSendAgent::TimeToSendPadding(size_t bytes,
	                                 const webrtc::PacedPacketInfo& cluster_info){
	NS_LOG_INFO("padding not implement");
	return bytes;
}
void TSendAgent::Bind(uint16_t port){
    if (socket_ == NULL) {
        socket_ = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (), port};
        auto res = socket_->Bind (local);
        NS_ASSERT (res == 0);
    }
    bind_port_=port;
    socket_->SetRecvCallback (MakeCallback(&TSendAgent::RecvPacket,this));

}
InetSocketAddress TSendAgent::GetLocalAddress(){
    Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal ();
	return InetSocketAddress{local_ip,bind_port_};
}
void TSendAgent::SetDestination(InetSocketAddress addr){
	peer_ip_=addr.GetIpv4();
	peer_port_=addr.GetPort();
}
void TSendAgent::StartApplication(){
	running_=true;
}
void TSendAgent::StopApplication(){
	running_=false;
	if(pm_!=NULL){
		pm_->Stop();
		if(send_bucket_!=NULL){
			pm_->DeRegisterModule(send_bucket_.get());
		}
	}
}
void TSendAgent::RecvPacket(Ptr<Socket> socket){
	Address remoteAddr;
    auto packet = socket->RecvFrom (remoteAddr);
    bin_stream_rewind(&stream_, 1);
    uint32_t len=packet->GetSize ();
	packet->CopyData ((uint8_t*)stream_.data,len);
	stream_.used=len;
	ProcessingMsg(&stream_);
}
void TSendAgent::SendToNetwork(uint8_t*data,uint32_t len){
	Ptr<Packet> p=Create<Packet>((uint8_t*)data,len);
	socket_->SendTo(p,0,InetSocketAddress{peer_ip_,peer_port_});
}
void TSendAgent::ProcessingMsg(bin_stream_t *stream){
	if(!running_){
		return;
	}
	sim_header_t header;
	if (sim_decode_header(stream, &header) != 0)
		return;
	uint8_t temp_pid=header.ver;
	if(temp_pid!=pid_){
		NS_LOG_ERROR("pid error");
		return;
	}
	if (header.mid < MIN_MSG_ID || header.mid > MAX_MSG_ID)
		return;
	switch(header.mid){
	case SIM_FEEDBACK:{
		sim_feedback_t feedback;
        //NS_LOG_INFO("recv feedback");
		if (sim_decode_msg(stream, &header, &feedback) != 0)
			return;
		IncomingFeedBack(&feedback);
        break;
	}
	default:
	{
		NS_LOG_ERROR("unknow message");
		break;
	}
	}
}
void TSendAgent::IncomingFeedBack(sim_feedback_t* feedback){
	std::unique_ptr<webrtc::rtcp::TransportFeedback> fb=
	webrtc::rtcp::TransportFeedback::ParseFrom((uint8_t*)feedback->feedback, feedback->feedback_size);
	transport_feedback_adapter_.OnTransportFeedback(*fb.get());
	std::vector<webrtc::PacketFeedback> feedback_vector = ReceivedPacketFeedbackVector(
	transport_feedback_adapter_.GetTransportFeedbackVector());
	SortPacketFeedbackVector(&feedback_vector);
    uint32_t now=Simulator::Now().GetMilliSeconds();
	for(auto it=feedback_vector.begin();it!=feedback_vector.end();it++){
		webrtc::PacketFeedback &info=(*it);
		if(!owd_cb_.IsNull()){
            uint16_t seq=info.sequence_number;
            uint32_t sent_ts=0;
            auto sent_it=sent_ts_map_.find(seq);
            if(sent_it!=sent_ts_map_.end()){
                sent_ts=sent_it->second;
                sent_ts_map_.erase(sent_it);
            }
			uint32_t delta=info.arrival_time_ms-info.send_time_ms;
            // there is a time offset in transport_feedback_adapter.cc
			owd_cb_(seq,sent_ts,info.arrival_time_ms,delta);
		}
	}

}
std::vector<webrtc::PacketFeedback> TSendAgent::ReceivedPacketFeedbackVector(
    const std::vector<webrtc::PacketFeedback>& input) {
  std::vector<webrtc::PacketFeedback> received_packet_feedback_vector;
  auto is_received = [](const webrtc::PacketFeedback& packet_feedback) {
    return packet_feedback.arrival_time_ms !=
           webrtc::PacketFeedback::kNotReceived;
  };
  std::copy_if(input.begin(), input.end(),
               std::back_inserter(received_packet_feedback_vector),
               is_received);
  return received_packet_feedback_vector;
}
void TSendAgent::SortPacketFeedbackVector(
    std::vector<webrtc::PacketFeedback>* const input) {
  RTC_DCHECK(input);
  std::sort(input->begin(), input->end(), webrtc::PacketFeedbackComparator());
}
void TSendAgent::OnFrame(uint8_t*data,uint32_t len){
	uint32_t timestamp=0;
	int64_t now=Simulator::Now().GetMilliSeconds();
	if(first_ts_==-1){
		first_ts_=now;
		ConfigurePace();
	}
	timestamp=now-first_ts_;
	uint8_t* pos=NULL;
	uint16_t splits[MAX_SPLIT_NUMBER], total=0;
	assert((len/ SIM_VIDEO_SIZE) < MAX_SPLIT_NUMBER);
	total = FrameSplit(splits, len);
	pos = (uint8_t*)data;
	sim_segment_t *seg=NULL;
	uint32_t i=0;
	for(i=0;i<total;i++){
		seg=new sim_segment_t();
		seg->fid = frame_seed_;
		seg->timestamp = timestamp;
		seg->ftype = 0;
		seg->payload_type =0;
		seg->index = i;
		seg->total = total;
		seg->remb = 1;
		seg->send_ts = 0;
		seg->transport_seq =trans_seq_;
		seg->data_size = splits[i];
		memcpy(seg->data, pos, seg->data_size);
		pos += splits[i];
		buf_.insert(std::make_pair(trans_seq_,seg));
		send_bucket_->InsertPacket(webrtc::PacedSender::kNormalPriority,uid_,
				trans_seq_,now,seg->data_size+SIM_SEGMENT_HEADER_SIZE,false);
		pending_len_+=seg->data_size+SIM_SEGMENT_HEADER_SIZE;
		packet_seed_++;
		trans_seq_++;
	}
	frame_seed_++;
	if(last_log_pending_ts_==0){
		last_log_pending_ts_=now;
	}
}
void TSendAgent::ConfigurePace(){
	pm_.reset(new ProcessModule());
	send_bucket_.reset(new TPacedSender(&clock_, this, nullptr));
	send_bucket_->SetEstimatedBitrate(rate_);
    send_bucket_->SetPacingFactor(1.2);
	send_bucket_->SetProbingEnabled(false);
	pm_->RegisterModule(send_bucket_.get(),RTC_FROM_HERE);
	pm_->Start();
}
}




