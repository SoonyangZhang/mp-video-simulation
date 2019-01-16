#include "path_sender_v1.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "net/third_party/quic/platform/api/quic_endian.h"
#include "net/third_party/quic/core/quic_data_reader.h"
#include "net/third_party/quic/core/quic_data_writer.h"
#include "net/my_bbr_sender.h"
#include "net/my_quic_header.h"
#include "ns3/ns_quic_time.h"
#include "ns3/bbr_sender_v1.h"
#define MAX_BUF_SIZE 1400
#define PADDING_SIZE 500
const uint8_t kPublicHeaderSequenceNumberShift = 4;
//insist factor 0.6 
//and disable congestion backoff mode
//change  updatagianphase
// close loss backoff
const uint32_t max_pacer_queue_delay=100;
namespace ns3{
NS_LOG_COMPONENT_DEFINE("PathSenderV1");
const int smooth_rate_num=90;
const int smooth_rate_den=100;
const int smooth_offset_num=90;
const int smooth_offset_den=100;
const int smooth_delay_num=85;
const int smooth_delay_den=100;
PathSenderV1::PathSenderV1(uint32_t min_bps,uint32_t max_bps)
:min_bps_(min_bps)
,max_bps_(max_bps){
//why
    cc_=new quic::MyBbrSenderV1(min_bps,this);
    quic::QuicBandwidth bw=quic::QuicBandwidth::FromBitsPerSecond(max_bps_);
	pacer_.set_sender(cc_);
    //pacer_.set_max_bps(2000000);
    //pacer_.set_max_pacing_rate(bw);
	bps_=500000;
    s_bps_=500000;
}
PathSenderV1::~PathSenderV1(){
    delete cc_;
}
void PathSenderV1::HeartBeat(){
	if(heart_timer_.IsExpired()){
        uint32_t now=Simulator::Now().GetMilliSeconds();
		//CheckQueueExceed(now);
        UpdateQueueOffset();
		ProcessVideoPacket();
        //SendFakePacket();//for test
        RecordRate(now);
        //UpdateRate(now);
		Time next=MilliSeconds(heart_beat_t_);
		heart_timer_=Simulator::Schedule(next,
				&PathSenderV1::HeartBeat,this);
	}
}
void PathSenderV1::ConfigureFps(uint32_t fps){
// 40 ms
	uint32_t ms=1000/fps;
//	cc_.UserDefinePeriod(ms);
}
void PathSenderV1::OnVideoPacket(uint32_t packet_id,
		std::shared_ptr<zsy::VideoPacketWrapper>packet,
		bool is_retrans){
    //return;
	uint32_t now=Simulator::Now().GetMilliSeconds();
	packet->set_enqueue_time(now);
	if(!is_retrans){
		sending_queue_.push_back(packet);
	}else{
		resending_queue_.push_back(packet);
	}
	pending_bytes_+=packet->get_payload_size();
	auto id_delay_it=id_delay_map.find(packet_id);
	if(id_delay_it==id_delay_map.end()){
		id_delay_map.insert(std::make_pair(packet_id,now));
	}else{
		//may not update the time stamp
	}
	if(first_packet_){
		first_packet_=false;
		//heart_timer_=Simulator::ScheduleNow(&PathSenderV1::HeartBeat,this);
	}
}
void PathSenderV1::OnAck(uint64_t seq){
	quic::QuicTime quic_now=Ns3QuicTime::Now();
	pacer_.OnCongestionEvent(quic_now,seq);
	UpdateRtt(seq);
	DetectLoss(seq);
}
void PathSenderV1::Bind(uint16_t port){
    if (socket_ == NULL) {
        socket_ = Socket::CreateSocket (GetNode (),UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (), port};
        auto res = socket_->Bind (local);
        NS_ASSERT (res == 0);
    }
    bind_port_=port;
    socket_->SetRecvCallback (MakeCallback(&PathSenderV1::RecvPacket,this));
}
InetSocketAddress PathSenderV1::GetLocalAddress(){
    Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal ();
	return InetSocketAddress{local_ip,bind_port_};
}
void PathSenderV1::ConfigurePeer(Ipv4Address addr,uint16_t port){
	peer_ip_=addr;
	peer_port_=port;
}
void PathSenderV1::OnBandwidthUpdate(){
		int64_t bw=0;
		bw=cc_->GetReferenceRate().ToKBitsPerSecond()*1000;
        //bps_=protection_rate;
        s_bps_=bw;
		//s_bps_=(smooth_rate_num*bw+(smooth_rate_den
		//		-smooth_rate_num)*s_bps_)/smooth_rate_den;
        uint64_t protection_rate=s_bps_*0.8;
		if(protection_rate<min_bps_){
			protection_rate=min_bps_;
		}
        bps_=protection_rate;
        if(mpsender_){
            mpsender_->OnRateUpdate();
        }    
}
void PathSenderV1::StartApplication(){
	//SendPingMessage();
    if(mpsender_){
		mpsender_->OnNetworkAvailable(pid_);
	}
	heart_timer_=Simulator::ScheduleNow(&PathSenderV1::HeartBeat,this);
}
void PathSenderV1::StopApplication(){
	if(!heart_timer_.IsExpired()){
		heart_timer_.Cancel();
	}
	if(mpsender_){
		mpsender_->OnNetworkStop(pid_);
	}

}
void PathSenderV1::ProcessVideoPacket(){
	quic::QuicTime quic_now=Ns3QuicTime::Now();
	if(pacer_.TimeUntilSend(quic_now)==quic::QuicTime::Delta::Zero()){
		if(!resending_queue_.empty()){
			SendStreamPacket(quic_now,true);
			return;
		}
		if(!sending_queue_.empty()){
			SendStreamPacket(quic_now,false);
			return;
		}
        if(cc_->ShouldSendProbePacket()){
            SendPaddingPacket(quic_now);
        }
	}
}
void PathSenderV1::SendPingMessage(){
	char buf[32];
    uint8_t public_flags=0;
	uint8_t type=zsy::Video_Proto_V1::v1_ping;
	quic::QuicDataWriter writer(32, buf, quic::NETWORK_BYTE_ORDER);
	uint32_t now=Simulator::Now().GetMilliSeconds();
    public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteUInt32(now);
	uint32_t len=writer.length();
	Ptr<Packet> p=Create<Packet>((uint8_t*)buf,len);
	SendToNetwork(p);
}
void PathSenderV1::RecvPacket(Ptr<Socket> socket){
	Address remoteAddr;
	auto packet = socket->RecvFrom (remoteAddr);
	uint32_t recv=packet->GetSize ();
	char *buf=new char[recv];
	packet->CopyData((uint8_t*)buf,recv);
	OnIncomingMessage((uint8_t*)buf,recv);
	delete buf;
}
void PathSenderV1::SendToNetwork(Ptr<Packet> p){
	socket_->SendTo(p,0,InetSocketAddress{peer_ip_,peer_port_});
}
void PathSenderV1::OnIncomingMessage(uint8_t *buf,uint32_t len){
	quic::QuicDataReader reader((char*)buf,len,  quic::NETWORK_BYTE_ORDER);
	uint8_t public_flags=0;
	reader.ReadBytes(&public_flags,1);
	uint8_t type=public_flags&0x0F;
	if(type==zsy::Video_Proto_V1::v1_pong){
		uint32_t sent_time=0;
		reader.ReadUInt32(&sent_time);
		uint32_t now=Simulator::Now().GetMilliSeconds();
		uint32_t rtt=now-sent_time;
		//NS_LOG_INFO("rtt "<<rtt);
	}
	if(type==zsy::Video_Proto_V1::v1_ack){
		quic::my_quic_header_t header;
		header.seq_len= quic::ReadSequenceNumberLength(
		        public_flags >> kPublicHeaderSequenceNumberShift);
		uint64_t num=0;
		reader.ReadBytesToUInt64(header.seq_len,&num);
		OnAck(num);
	}
}
void PathSenderV1::SendFakePacket(){
 	quic::QuicTime quic_now=Ns3QuicTime::Now();
	if(pacer_.TimeUntilSend(quic_now)==quic::QuicTime::Delta::Zero()){
        SendPaddingPacket(quic_now);
    }   
}
void PathSenderV1::SendPaddingPacket(quic::QuicTime quic_now){
	char buf[MAX_BUF_SIZE]={0};
    uint32_t ms=Simulator::Now().GetMilliSeconds();
	quic::my_quic_header_t header;
	header.seq=seq_;
	header.seq_len=quic::GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= quic::GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;

	quic::QuicDataWriter writer(MAX_BUF_SIZE, buf, quic::NETWORK_BYTE_ORDER);
	uint8_t type=zsy::Video_Proto_V1::v1_padding;
	public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteBytesToUInt64(header.seq_len,header.seq);
    writer.WriteUInt32(ms);
	seq_++;
	Ptr<Packet> p=Create<Packet>((uint8_t*)buf,PADDING_SIZE);
	SendToNetwork(p);
	uint16_t payload=PADDING_SIZE-(1+header.seq_len);
	pacer_.OnPacketSent(quic_now,header.seq,payload);
	seq_delay_map_.insert(std::make_pair(header.seq,ms));
	//0 mean this packet without effective packet id;
	seq_id_map_.insert(std::make_pair(header.seq,0));
}
void PathSenderV1::SendStreamPacket(quic::QuicTime quic_now,bool is_retrans){
	char buf[MAX_BUF_SIZE]={0};
	quic::my_quic_header_t header;
	header.seq=seq_;
	header.seq_len=quic::GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= quic::GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;

	quic::QuicDataWriter writer(MAX_BUF_SIZE, buf, quic::NETWORK_BYTE_ORDER);
	uint8_t type=zsy::Video_Proto_V1::v1_stream;
	public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteBytesToUInt64(header.seq_len,header.seq);
	seq_++;
	int payload_size=0;
	uint32_t video_size=0;
	uint32_t packet_id=0;
	uint32_t ns_now=Simulator::Now().GetMilliSeconds();
	char seg_buf[MAX_BUF_SIZE];
	if(is_retrans){
		auto resend_it=resending_queue_.begin();
		std::shared_ptr<zsy::VideoPacketWrapper> packet=(*resend_it);
		packet_id=packet->get_packet_id();
		video_size=packet->video_size();
		payload_size=packet->Serialize((uint8_t*)seg_buf,MAX_BUF_SIZE,ns_now);
		uint32_t time_offset=packet->get_time_offset();
		float tmp_offset=((float)(queue_offset_*(smooth_offset_den-smooth_offset_num)+smooth_offset_num*time_offset))/smooth_offset_den;
		queue_offset_=tmp_offset;
		resending_queue_.erase(resend_it);
		if(!trace_time_offset_cb_.IsNull()){
			trace_time_offset_cb_(packet_id,time_offset);
		}
	}else{
		auto send_it=sending_queue_.begin();
		std::shared_ptr<zsy::VideoPacketWrapper> packet=(*send_it);
		packet_id=packet->get_packet_id();
		video_size=packet->video_size();
		payload_size=packet->Serialize((uint8_t*)seg_buf,MAX_BUF_SIZE,ns_now);
		uint32_t time_offset=packet->get_time_offset();
		float tmp_offset=((float)(queue_offset_*(smooth_offset_den-smooth_offset_num)+smooth_offset_num*time_offset))/smooth_offset_den;
		queue_offset_=tmp_offset;
		sending_queue_.erase(send_it);
		if(!trace_time_offset_cb_.IsNull()){
			trace_time_offset_cb_(packet_id,time_offset);
		}
	}
	assert(payload_size!=-1);
	pending_bytes_-=payload_size;
	writer.WriteBytes(seg_buf,payload_size);
	uint32_t length=writer.length();
	Ptr<Packet> p=Create<Packet>((uint8_t*)buf,length);
	SendToNetwork(p);
	pacer_.OnPacketSent(quic_now,header.seq,video_size);
	seq_delay_map_.insert(std::make_pair(header.seq,ns_now));
	seq_id_map_.insert(std::make_pair(header.seq,packet_id));
}
void PathSenderV1::RecordRate(uint32_t now){
	if(!trace_rate_cb_.IsNull()){
		if(rate_out_next_==0){
			rate_out_next_=now+100;
			return;
		}
		if(now>=rate_out_next_){
			int64_t bw=0;
			bw=cc_->GetReferenceRate().ToKBitsPerSecond()*1000;
            //NS_LOG_INFO("rate "<<bw);
			trace_rate_cb_(bw);
			rate_out_next_=now+100;
		}
	}else{
    }
}
void PathSenderV1::UpdateRate(uint32_t now){
	if(cur_rtt_==0){
		rate_update_next_=now;
		return;
	}
	if(now>=rate_update_next_){
		int64_t bw=0;
		bw=cc_->GetReferenceRate().ToKBitsPerSecond()*1000;
		if(bw<min_bps_){
			bw=min_bps_;
		}
        bps_=bw;
		//bps_=(smooth_rate_num*bw+(smooth_rate_den
		//		-smooth_rate_num)*bps_)/smooth_rate_den;
		rate_update_next_=now+cur_rtt_;
        //NS_LOG_INFO("refer "<<bps_);
        if(mpsender_){
            mpsender_->OnRateUpdate();
        }
	}
}
void  PathSenderV1::UpdateRtt(uint64_t seq){
	uint32_t ns_now=Simulator::Now().GetMilliSeconds();
	auto it=seq_delay_map_.find(seq);
	if(it!=seq_delay_map_.end()){
		cur_rtt_=ns_now-it->second;
	}
	auto seq_id_it=seq_id_map_.find(seq);
	if(seq_id_it!=seq_id_map_.end()){
		uint32_t packet_id=seq_id_it->second;
		auto id_delay_it=id_delay_map.find(packet_id);
		if(id_delay_it!=id_delay_map.end()){
			uint32_t temp_delay=ns_now-id_delay_it->second;
			UpdateAggregeteDelay(temp_delay);
			RemoveIdDelayMapLe(packet_id);
		}
	}
	RemoveSeqDelayMapLe(seq);
}
void PathSenderV1::UpdateAggregeteDelay(uint32_t delay){
	if(aggregate_delay_==0){
		aggregate_delay_=delay;
	}else{
		aggregate_delay_=((smooth_delay_den-smooth_delay_num)*aggregate_delay_
				+smooth_delay_num*delay)/smooth_delay_den;
	}
}
void PathSenderV1::DetectLoss(uint64_t seq){
	if(seq>=recv_seq_+1){
		uint64_t i=recv_seq_+1;
		uint32_t rtt=cur_rtt_;
		for(;i<seq;i++){
			if(!trace_loss_cb_.IsNull()){
				trace_loss_cb_(i,rtt);
			}
			auto seq_id_it=seq_id_map_.find(i);
			if(seq_id_it!=seq_id_map_.end()){
				uint32_t packet_id=seq_id_it->second;
				if(packet_id!=0){
                    if(mpsender_){
                        mpsender_->OnPacketLost(pid_,packet_id);
                    }	
				}
			}
		}
		recv_seq_=seq;
	}
	RemoveSeqIdMapLe(seq);
}
void PathSenderV1::RemoveSeqIdMapLe(uint64_t seq){
	while(!seq_id_map_.empty()){
		auto it=seq_id_map_.begin();
		if(it->first<=seq){
			seq_id_map_.erase(it);
		}else{
			break;
		}
	}
}
void PathSenderV1::RemoveSeqDelayMapLe(uint64_t seq){
	while(!seq_delay_map_.empty()){
		auto it=seq_delay_map_.begin();
		if(it->first<=seq){
			seq_delay_map_.erase(it);
		}else{
			break;
		}
	}
}
void PathSenderV1::RemoveIdDelayMapLe(uint32_t packet_id){
	while(!id_delay_map.empty()){
		auto it=id_delay_map.begin();
		if(it->first<=packet_id){
			id_delay_map.erase(it);
		}else{
			break;
		}
	}
}
void PathSenderV1::CheckQueueExceed(uint32_t now){
	bool has_over_queue_packet_=false;
	auto resending_queue_it=resending_queue_.begin();
	if(resending_queue_it!=resending_queue_.end()){
		std::shared_ptr<zsy::VideoPacketWrapper> packet=(*resending_queue_it);
		uint32_t queue_delay=packet->get_queue_delay(now);
		if(queue_delay>max_pacer_queue_delay){
			has_over_queue_packet_=true;
		}
	}
	auto sending_queue_it=sending_queue_.begin();
	if(sending_queue_it!=sending_queue_.end()){
		std::shared_ptr<zsy::VideoPacketWrapper> packet=(*sending_queue_it);
		uint32_t queue_delay=packet->get_queue_delay(now);
		if(queue_delay>max_pacer_queue_delay){
			has_over_queue_packet_=true;
		}
	}
	if(has_over_queue_packet_){
		if(now-over_offset_ts_>cur_rtt_){
			bps_=bps_*0.8;
			over_offset_ts_=now;
	        if(mpsender_){
	            mpsender_->OnRateUpdate();
	        }
		}
	}
	/*while(!resending_queue_.empty()){
		auto it=resending_queue_.begin();
		std::shared_ptr<zsy::VideoPacketWrapper> packet=(*it);
		uint32_t queue_delay=packet->get_queue_delay(now);
		if(queue_delay>max_pacer_queue_delay){
			SendPacketWithoutCongestion(packet,now);
			resending_queue_.erase(it);
		}else{
			break;
		}
	}
	while(!sending_queue_.empty()){
		auto it=sending_queue_.begin();
		std::shared_ptr<zsy::VideoPacketWrapper> packet=(*it);
		uint32_t queue_delay=packet->get_queue_delay(now);
		if(queue_delay>max_pacer_queue_delay){
			SendPacketWithoutCongestion(packet,now);
			sending_queue_.erase(it);
		}else{
			break;
		}
	}*/
}
void PathSenderV1::SendPacketWithoutCongestion(std::shared_ptr<zsy::VideoPacketWrapper>packet,
		uint32_t ns_now){
	char buf[MAX_BUF_SIZE]={0};
	char seg_buf[MAX_BUF_SIZE];
	quic::my_quic_header_t header;
	header.seq=seq_;
	header.seq_len=quic::GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= quic::GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;

	quic::QuicDataWriter writer(MAX_BUF_SIZE, buf, quic::NETWORK_BYTE_ORDER);
	uint8_t type=zsy::Video_Proto_V1::v1_stream;
	public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteBytesToUInt64(header.seq_len,header.seq);
	seq_++;
	int payload_size=0;
	uint32_t video_size=0;
	uint32_t packet_id=0;
	packet_id=packet->get_packet_id();
	video_size=packet->video_size();
	payload_size=packet->Serialize((uint8_t*)seg_buf,MAX_BUF_SIZE,ns_now);
	uint32_t time_offset=packet->get_time_offset();
	if(!trace_time_offset_cb_.IsNull()){
		trace_time_offset_cb_(packet_id,time_offset);
	}
	assert(payload_size!=-1);
	pending_bytes_-=payload_size;
	writer.WriteBytes(seg_buf,payload_size);
	uint32_t length=writer.length();
	Ptr<Packet> p=Create<Packet>((uint8_t*)buf,length);
	SendToNetwork(p);
	seq_delay_map_.insert(std::make_pair(header.seq,ns_now));
	seq_id_map_.insert(std::make_pair(header.seq,packet_id));
}
void PathSenderV1::UpdateQueueOffset(){
	if(resending_queue_.empty()&&sending_queue_.empty()){
		queue_offset_=0;
	}
}
}
