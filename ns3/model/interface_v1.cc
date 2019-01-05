#include "ns3/interface_v1.h"
#include "ns3/path_sender_v1.h"
using namespace ns3;
namespace zsy{
void ScheduleV1::RetransPackets(uint32_t packet_id,uint8_t origin_pid){
	uint32_t total_path=pids_.size();
	if(total_path==1){
		sender_->PacketRetrans(packet_id,origin_pid);
	}else{
		uint32_t min_rtt=0xFFFFFFFF;
		uint8_t min_pid=0;
		for(auto it=pids_.begin();it!=pids_.end();it++){
			uint8_t path_id=(*it);
			if(path_id==origin_pid){
				continue;
			}
			uint32_t rtt=0;
			Ptr<PathSenderV1> path=sender_->GetPathInfo(path_id);
			rtt=path->get_rtt();
			if(rtt<min_rtt){
				min_rtt=rtt;
				min_pid=path_id;
			}
		}
		sender_->PacketRetrans(packet_id,min_pid);
	}
}
void ScheduleV1::SendAllPackesToPath(std::map<uint32_t,uint32_t>&packets,uint8_t pid){
	for(auto it=packets.begin();it!=packets.end();it++){
		uint32_t packet_id=it->first;
		sender_->PacketSchedule(packet_id,pid);
	}
}
void ScheduleV1::RegisterPath(uint8_t  pid){
	for(auto it=pids_.begin();it!=pids_.end();it++){
		if((*it)==pid){
			return;
		}
	}
	pids_.push_back(pid);
}
void ScheduleV1::UnregisterPath(uint8_t pid){
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
}
