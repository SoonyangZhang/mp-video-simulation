#include "ns3/mock_receiver.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "net/third_party/quic/platform/api/quic_endian.h"
#include "net/third_party/quic/core/quic_data_reader.h"
#include "net/third_party/quic/core/quic_data_writer.h"
#include "net/my_quic_header.h"
#define MAX_BUF_SIZE 1400
const uint8_t kPublicHeaderSequenceNumberShift = 4;
namespace ns3{
NS_LOG_COMPONENT_DEFINE("MockReceiver");
void MockReceiver::Bind(uint16_t port){
    if (socket_ == NULL) {
        socket_ = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (), port};
        auto res = socket_->Bind (local);
        NS_ASSERT (res == 0);
    }
    bind_port_=port;
    socket_->SetRecvCallback (MakeCallback(&MockReceiver::RecvPacket,this));
}
InetSocketAddress MockReceiver::GetLocalAddress(){
    Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal ();
	return InetSocketAddress{local_ip,bind_port_};
}
void MockReceiver::StartApplication(){

}
void MockReceiver::StopApplication(){

}
void MockReceiver::RecvPacket(Ptr<Socket> socket){
	Address remoteAddr;
	auto packet = socket->RecvFrom (remoteAddr);
	if(!know_peer_){
        peer_ip_ = InetSocketAddress::ConvertFrom (remoteAddr).GetIpv4 ();
	    peer_port_= InetSocketAddress::ConvertFrom (remoteAddr).GetPort ();
	    know_peer_=true;
	}
	uint32_t recv=packet->GetSize ();
	char *buf=new char[recv];
	packet->CopyData((uint8_t*)buf,recv);
	OnIncomingMessage((uint8_t*)buf,recv);
	delete buf;
}
void MockReceiver::OnIncomingMessage(uint8_t *buf,uint32_t len){
	quic::QuicDataReader reader((char*)buf,len,  quic::NETWORK_BYTE_ORDER);
	uint8_t public_flags=0;
	reader.ReadBytes(&public_flags,1);
	uint8_t type=public_flags&0x0F;
	uint32_t ns_now=Simulator::Now().GetMilliSeconds();
	uint32_t owd=0;
	if(type==zsy::Video_Proto_V1::v1_ping){
		uint32_t sent_ts=0;
		reader.ReadUInt32(&sent_ts);
		SendPong(sent_ts);
		owd=ns_now-sent_ts;
		UpdateOneWayDelay(owd);
	}
	if(type==zsy::Video_Proto_V1::v1_stream||type==zsy::Video_Proto_V1::v1_padding){
		quic::my_quic_header_t header;
		header.seq_len= quic::ReadSequenceNumberLength(
	        public_flags >> kPublicHeaderSequenceNumberShift);
		uint64_t seq;
		reader.ReadBytesToUInt64(header.seq_len,&seq);
		SendAck(seq);
		if(type==zsy::Video_Proto_V1::v1_stream){
			uint32_t offset=1+header.seq_len;
			uint32_t video_len=len-offset;
			uint8_t *video_buf=buf+offset;
			int64_t first_ts=-1;//sender_->get_first_ts();
			std::shared_ptr<zsy::VideoPacketReceiveWrapper> packet(
					new zsy::VideoPacketReceiveWrapper(ns_now));
			packet->Deserialize(video_buf,video_len);
			if(first_ts!=-1){
				packet->set_first_ts(first_ts);
				owd=packet->get_owd();
				UpdateOneWayDelay(owd);
			}
			ProcessVideoPacket(packet);
		}
		if(type==zsy::Video_Proto_V1::v1_padding){
            //NS_LOG_INFO("padding"<<seq);
			uint32_t sent_ts=0;
			reader.ReadUInt32(&sent_ts);
			owd=ns_now-sent_ts;
			UpdateOneWayDelay(owd);
		}
	}
}
void MockReceiver::SendPong(uint32_t sent_ts){
	NS_LOG_INFO("recv ping "<<sent_ts);
	char buf[32];
    uint8_t public_flags=0;
	uint8_t type=zsy::Video_Proto_V1::v1_pong;
	quic::QuicDataWriter writer(32, buf, quic::NETWORK_BYTE_ORDER);
    public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteUInt32(sent_ts);
	uint32_t len=writer.length();
	Ptr<Packet> p=Create<Packet>((uint8_t*)buf,len);
	SendToNetwork(p);
}
void MockReceiver::SendAck(uint64_t seq){
	char buf[MAX_BUF_SIZE]={0};
	quic::my_quic_header_t header;
	header.seq=seq;
	header.seq_len=quic::GetMinSeqLength(header.seq);
	uint8_t public_flags=0;
	public_flags |= quic::GetPacketNumberFlags(header.seq_len)
                  << kPublicHeaderSequenceNumberShift;
	quic::QuicDataWriter writer(MAX_BUF_SIZE, buf, quic::NETWORK_BYTE_ORDER);
	uint8_t type=zsy::Video_Proto_V1::v1_ack;
	public_flags|=type;
	writer.WriteBytes(&public_flags,1);
	writer.WriteBytesToUInt64(header.seq_len,header.seq);
	uint32_t len=writer.length();
	Ptr<Packet> p=Create<Packet>((uint8_t*)buf,len);
	SendToNetwork(p);
}
void MockReceiver::SendToNetwork(Ptr<Packet> p){
	socket_->SendTo(p,0,InetSocketAddress{peer_ip_,peer_port_});
}
void MockReceiver::UpdateOneWayDelay(uint32_t new_owd){
	if(owd_==0){
		owd_=new_owd;
	}else{
		owd_var_=(owd_var_*3+std::abs(new_owd-owd_))/4;
		owd_=(7*owd_+new_owd)/8;
	}
}
void MockReceiver::ProcessVideoPacket(std::shared_ptr<zsy::VideoPacketReceiveWrapper> packet){

}
}




