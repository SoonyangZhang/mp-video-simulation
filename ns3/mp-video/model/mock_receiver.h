#ifndef NS3_MPVIDEO_MOCK_RECEIVER_H_
#define NS3_MPVIDEO_MOCK_RECEIVER_H_

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
class MockReceiver:public Application{
public:
	void Bind(uint16_t port);
	InetSocketAddress GetLocalAddress();
	typedef Callback<void,uint64_t,uint32_t>TraceOwd;
	void SetDelayTraceFunc(TraceOwd cb){
		trace_owd_cb_=cb;
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
    void NewSeqDelay(uint64_t seq,uint32_t owd);
	Ipv4Address peer_ip_;
    uint16_t peer_port_;
    Ptr<Socket> socket_;
    uint16_t bind_port_;
    uint32_t  owd_{0};
    uint32_t owd_var_{5};
    bool know_peer_{false};
    TraceOwd trace_owd_cb_;
};
}
#endif /* NS3_MPVIDEO_MOCK_RECEIVER_H_ */
