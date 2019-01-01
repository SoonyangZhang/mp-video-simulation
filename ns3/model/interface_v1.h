#ifndef NS3_MPVIDEO_INTERFACE_V1_H_
#define NS3_MPVIDEO_INTERFACE_V1_H_
#include "ns3/ptr.h"
#include "video_proto.h"
#include "ns3/path_sender_v1.h"
#include <map>
#include <vector>
#include <memory>
namespace zsy{
class MpSenderIntefaceV1{
public:
	virtual ~MpSenderIntefaceV1(){}
	virtual void PacketSchedule(uint32_t packet_id,uint8_t pid)=0;
	virtual void PacketRetrans(uint32_t packet_id,uint8_t pid)=0;
	virtual int64_t GetFirstTs()=0;
	virtual ns3::Ptr<ns3::PathSenderV1> GetPathInfo(uint8_t pid)=0;
};
class ScheduleV1{
public:
	virtual ~ScheduleV1(){}
	void SetSender(MpSenderIntefaceV1* s){sender_=s;}
	//std::map<uint32_t,uint32_t>&packets  packet id ->length
	virtual void IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size)=0;
	virtual void RetransPackets(uint32_t packet_id,uint8_t origin_pid)=0;
	virtual void SendAllPackesToPath(std::map<uint32_t,uint32_t>&packets,uint8_t pid)=0;
	void RegisterPath(uint8_t  pid){
		for(auto it=pids_.begin();it!=pids_.end();it++){
			if((*it)==pid){
				return;
			}
		}
		pids_.push_back(pid);
	}
	void UnregisterPath(uint8_t pid){
		for(auto it=pids_.begin();it!=pids_.end();){
			if((*it)==pid){
				it=pids_.erase(it);
				break;
			}
			else{
				it++;
			}
		}
	}
protected:
    std::vector<uint8_t> pids_;
    MpSenderIntefaceV1 *sender_;
};
class MpReceiverInterfaceV1{
public:
	virtual ~MpReceiverInterfaceV1(){}
	virtual void OnVideoPacket(std::shared_ptr<VideoPacketReceiveWrapper> packet)=0;
	virtual void StopReceiver()=0;
};
}




#endif /* NS3_MPVIDEO_INTERFACE_V1_H_ */
