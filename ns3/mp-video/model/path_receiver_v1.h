#ifndef NS3_MPVIDEO_PATH_RECEIVER_V1_H_
#define NS3_MPVIDEO_PATH_RECEIVER_V1_H_
#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/event-id.h"
#include "ns3/callback.h"
#include "video_proto.h"
#include "ns3/callback.h"
#include "ns3/interface_v1.h"
#include <memory>
namespace ns3{
class PathReceiverV1:public Application{
public:
	void Bind(uint16_t port);
	InetSocketAddress GetLocalAddress();
	void RegisterMpSink(zsy::MpReceiverInterfaceV1 *sink){
		mpsink_=sink;
	}
	void RegisterPathSender(Ptr<PathSenderV1> sender){
		sender_=sender;
	}
	void set_path_id(uint8_t pid){
		pid_=pid;
	}
	uint8_t get_path_id(){
		return pid_;
	}
private:
	virtual void StartApplication() override;
	virtual void StopApplication() override;
	void RecvPacket(Ptr<Socket> socket);
	void OnIncomingMessage(uint8_t *buf,uint32_t len);
	void SendPong(uint32_t sent_ts);
	void SendAck(uint64_t seq);
	void SendToNetwork(Ptr<Packet> p);
	void UpdateOneWayDelay(uint32_t new_owd);
	void ProcessVideoPacket(std::shared_ptr<zsy::VideoPacketReceiveWrapper> packet);
	bool know_peer_{false};
    Ipv4Address peer_ip_;
    uint16_t peer_port_;
    Ptr<Socket> socket_;
    uint16_t bind_port_;
    uint32_t  owd_{0};
    uint32_t owd_var_{5};
    zsy::MpReceiverInterfaceV1 *mpsink_{NULL};
    uint8_t pid_{0};
    Ptr<PathSenderV1> sender_{NULL};
};
}




#endif /* NS3_MPVIDEO_PATH_RECEIVER_V1_H_ */
