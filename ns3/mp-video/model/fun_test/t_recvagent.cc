#include "t_recvagent.h"
#include "rtc_base/location.h"
#include "ns3/log.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE("TRecvAgent");
TRecvAgent::TRecvAgent(){
	bin_stream_init(&stream_);
}
TRecvAgent::~TRecvAgent(){
	bin_stream_destroy(&stream_);
}
void TRecvAgent::Bind(uint16_t port){
    if (socket_ == NULL) {
        socket_ = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
        auto local = InetSocketAddress{Ipv4Address::GetAny (), port};
        auto res = socket_->Bind (local);
        NS_ASSERT (res == 0);
    }
    bind_port_=port;
    socket_->SetRecvCallback (MakeCallback(&TRecvAgent::RecvPacket,this));
}
InetSocketAddress TRecvAgent::GetLocalAddress(){
    Ptr<Node> node=GetNode();
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    Ipv4Address local_ip = ipv4->GetAddress (1, 0).GetLocal ();
	return InetSocketAddress{local_ip,bind_port_};
}
bool TRecvAgent::SendTransportFeedback(webrtc::rtcp::TransportFeedback* packet){
	packet->SetSenderSsrc(uid_);
	rtc::Buffer serialized= packet->Build();
	//sim_header_t header;
	//sim_feedback_t feedback;
    uint32_t payload_size=serialized.size();
	if (payload_size > SIM_FEEDBACK_SIZE){
		NS_LOG_ERROR("feedback size > SIM_FEEDBACK_SIZE");
		return false;
	}
	sim_header_t header;
	sim_feedback_t feedback;
	INIT_SIM_HEADER(header, SIM_FEEDBACK, uid_);
    header.ver=pid_;
	feedback.base_packet_id =0;
	feedback.feedback_size = payload_size;
	memcpy(feedback.feedback, (uint8_t*)serialized.data(), payload_size);
	sim_encode_msg(&stream_, &header, &feedback);
	SendToNetwork(stream_.data,stream_.used);
	return true;
}
void TRecvAgent::StartApplication(){
	running_=true;
}
void TRecvAgent::StopApplication(){
	running_=false;
	if(pm_!=NULL){
		if(remote_estimator_proxy_!=NULL){
			pm_->DeRegisterModule(remote_estimator_proxy_.get());
		}
	}
}
void TRecvAgent::RecvPacket(Ptr<Socket> socket){
	Address remoteAddr;
    auto packet = socket->RecvFrom (remoteAddr);
    //NS_LOG_INFO("incoming ");
    if(first_packet_){
        peer_ip_ = InetSocketAddress::ConvertFrom (remoteAddr).GetIpv4 ();
	    peer_port_= InetSocketAddress::ConvertFrom (remoteAddr).GetPort ();
    	ConfigureRemoteProxy();
	    first_packet_=false;
    }
    bin_stream_rewind(&stream_, 1);
    uint32_t len=packet->GetSize ();
	packet->CopyData ((uint8_t*)stream_.data,len);
	stream_.used=len;
	ProcessingMsg(&stream_);
}
void TRecvAgent::ProcessingMsg(bin_stream_t *stream){
	if(!running_){
		return;
	}
	sim_header_t header;
	if (sim_decode_header(stream, &header) != 0)
		return;
	if (header.mid < MIN_MSG_ID || header.mid > MAX_MSG_ID)
		return;
	switch(header.mid){
	case SIM_SEG:{
		pid_=header.ver;
		sim_segment_t body;
		if (sim_decode_msg(stream, &header, &body) != 0){
			return;
		}
		OnReceiveSegment(&header,&body);
		break;
	}
	default:{
		NS_LOG_ERROR("unknown message");
		break;
	}
	}
}
void TRecvAgent::ConfigureRemoteProxy(){
	if(pm_==NULL){
		pm_.reset(new ProcessModule());
		if(remote_estimator_proxy_==NULL){
			remote_estimator_proxy_.reset(
					new webrtc::RemoteEstimatorProxy(&clock_,this));
		}
		pm_->RegisterModule(remote_estimator_proxy_.get(),RTC_FROM_HERE);
		pm_->Start();
	}

}
void TRecvAgent::OnReceiveSegment(sim_header_t *header,sim_segment_t *seg){
	uint32_t now=Simulator::Now().GetMilliSeconds();
	uint16_t seq=seg->transport_seq;
	if(!recv_ts_cb_.IsNull()){
		recv_ts_cb_(seq,now);
	}
    //NS_LOG_INFO("recv seg"<<now);
	uint32_t overhead=seg->data_size + SIM_SEGMENT_HEADER_SIZE;
	webrtc::RTPHeader fakeHeader;
	fakeHeader.ssrc=header->uid;
	fakeHeader.extension.hasTransportSequenceNumber=true;
	fakeHeader.extension.transportSequenceNumber=seq;
	remote_estimator_proxy_->IncomingPacket(now,overhead,fakeHeader);
}
void TRecvAgent::SendToNetwork(uint8_t*data,uint32_t len){
	Ptr<Packet> p=Create<Packet>((uint8_t*)data,len);
	socket_->SendTo(p,0,InetSocketAddress{peer_ip_,peer_port_});
}
}

