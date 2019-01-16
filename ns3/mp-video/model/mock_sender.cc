#include "ns3/mock_sender.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "net/third_party/quic/platform/api/quic_endian.h"
#include "net/third_party/quic/core/quic_data_reader.h"
#include "net/third_party/quic/core/quic_data_writer.h"
#include "net/my_bbr_sender.h"
#include "net/my_quic_header.h"
#include "ns3/ns_quic_time.h"
#include "ns3/bbr_sender_v1.h"
#include "ns3/bbr_sender_v2.h"
#include "ns3/bbr_sender_v3.h"
#include "ns3/bbr_sender_v4.h"
#define MAX_BUF_SIZE 1400
#define PADDING_SIZE 500
const uint8_t kPublicHeaderSequenceNumberShift = 4;
namespace ns3{
NS_LOG_COMPONENT_DEFINE("MockSender");
enum CC_VERSION{
    cc_v0,
    cc_v1,
    cc_v2,
    cc_v3,
    cc_v4,
};
MockSender::MockSender(uint32_t min_bps,uint32_t max_bps,int instance,uint8_t cc_ver)
:min_bps_(min_bps)
,max_bps_(max_bps){
    std::string test_case=std::to_string(instance);
    std::string log=std::string("bbr-aimd-")+test_case;
    tracer_.OpenTraceStateFile(log);
    if(cc_ver==cc_v1){
       quic::MyBbrSenderV1 *child_cc=NULL;
       child_cc=new quic::MyBbrSenderV1(min_bps,this); 
       cc_=child_cc;
    }else if(cc_ver==cc_v2){
        quic::MyBbrSenderV2 *child_cc=NULL;
        child_cc=new quic::MyBbrSenderV2(min_bps,this);
        cc_=child_cc;
        child_cc->SetStateTraceFunc(MakeCallback(&TraceState::OnNewState,&tracer_));
    }else if(cc_ver==cc_v3){
        quic::MyBbrSenderV3 *child_cc=NULL;
        child_cc=new quic::MyBbrSenderV3(min_bps,this);
        cc_=child_cc;
        child_cc->SetStateTraceFunc(MakeCallback(&TraceState::OnNewState,&tracer_));
    }else{
        quic::MyBbrSenderV4 *child_cc=NULL;
        child_cc=new quic::MyBbrSenderV4(min_bps,this);
        cc_=child_cc;
        child_cc->SetStateTraceFunc(MakeCallback(&TraceState::OnNewState,&tracer_));    
    }
	pacer_.set_sender(cc_);
    //quic::QuicBandwidth bw=quic::QuicBandwidth::FromBitsPerSecond(max_bps_);
    //pacer_.set_max_pacing_rate(bw);
	bps_=500000;
}
MockSender::~MockSender(){
	delete cc_;
}
void MockSender::HeartBeat(){
	if(heart_timer_.IsExpired()){
        uint32_t now=Simulator::Now().GetMilliSeconds();
        SendFakePacket();
		Time next=MilliSeconds(heart_beat_t_);
		heart_timer_=Simulator::Schedule(next,
				&MockSender::HeartBeat,this);
	}
}
void MockSender::OnAck(uint64_t seq){
	quic::QuicTime quic_now=Ns3QuicTime::Now();
	pacer_.OnCongestionEvent(quic_now,seq);
    //NS_LOG_INFO("ack "<<seq);
	UpdateRtt(seq);
	DetectLoss(seq);
}
void MockSender::Bind(uint16_t port){
    if (socket_ == NULL) {
        socket_ = Socket::CreateSocket (GetNode (),UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (), port};
        auto res = socket_->Bind (local);
        NS_ASSERT (res == 0);
    }
    bind_port_=port;
    socket_->SetRecvCallback (MakeCallback(&MockSender::RecvPacket,this));
}
InetSocketAddress MockSender::GetLocalAddress(){
    Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal ();
	return InetSocketAddress{local_ip,bind_port_};
}
void MockSender::ConfigurePeer(Ipv4Address addr,uint16_t port){
	peer_ip_=addr;
	peer_port_=port;
}
void MockSender::OnBandwidthUpdate(){
	int64_t bw=0;
	bw=cc_->GetReferenceRate().ToKBitsPerSecond()*1000;
	if(!trace_rate_cb_.IsNull()){
		trace_rate_cb_(bw);
	}
}
void MockSender::StartApplication(){
	heart_timer_=Simulator::ScheduleNow(&MockSender::HeartBeat,this);
}
void MockSender::StopApplication(){
	if(!heart_timer_.IsExpired()){
		heart_timer_.Cancel();
	}
}
void MockSender::RecvPacket(Ptr<Socket> socket){
	Address remoteAddr;
	auto packet = socket->RecvFrom (remoteAddr);
	uint32_t recv=packet->GetSize ();
	char *buf=new char[recv];
	packet->CopyData((uint8_t*)buf,recv);
	OnIncomingMessage((uint8_t*)buf,recv);
	delete buf;
}
void MockSender::SendToNetwork(Ptr<Packet> p){
	socket_->SendTo(p,0,InetSocketAddress{peer_ip_,peer_port_});
}
void MockSender::OnIncomingMessage(uint8_t *buf,uint32_t len){
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
void MockSender::SendFakePacket(){
 	quic::QuicTime quic_now=Ns3QuicTime::Now();
	if(pacer_.TimeUntilSend(quic_now)==quic::QuicTime::Delta::Zero()){
        SendPaddingPacket(quic_now);
    }
}
void MockSender::SendPaddingPacket(quic::QuicTime quic_now){
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
}
void  MockSender::UpdateRtt(uint64_t seq){
	uint32_t ns_now=Simulator::Now().GetMilliSeconds();
	auto it=seq_delay_map_.find(seq);
	if(it!=seq_delay_map_.end()){
		cur_rtt_=ns_now-it->second;
	}
	RemoveSeqDelayMapLe(seq);
}
void MockSender::DetectLoss(uint64_t seq){
	if(seq>=recv_seq_+1){
		uint64_t i=recv_seq_+1;
		uint32_t rtt=cur_rtt_;
		for(;i<seq;i++){
			if(!trace_loss_cb_.IsNull()){
				trace_loss_cb_(i,rtt);
			}
		}
		recv_seq_=seq;
	}
}
void MockSender::RemoveSeqDelayMapLe(uint64_t seq){
	while(!seq_delay_map_.empty()){
		auto it=seq_delay_map_.begin();
		if(it->first<=seq){
			seq_delay_map_.erase(it);
		}else{
			break;
		}
	}
}
}



