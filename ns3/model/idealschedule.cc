#include "idealschedule.h"
#include "ns3/wrr.h"
#include "ns3/path_sender_v1.h"
#include "ns3/simulator.h"
using namespace ns3;
namespace zsy{
void IdealSchedule::IncomingPackets(std::map<uint32_t,uint32_t>&packets,
		uint32_t size){
	uint32_t packet_num=packets.size();
	if(pids_.size()==1){
		uint8_t path_id=pids_[0];
		SendAllPackesToPath(packets,path_id);
	}else{
        uint32_t totalrate=0;
        std::map<uint8_t,uint32_t> pathrate;
		for(auto it=pids_.begin();it!=pids_.end();it++){
			uint8_t path_id=(*it);
			Ptr<PathSenderV1> path=sender_->GetPathInfo(path_id);
	        uint32_t rate=0;
			if(rate!=0){
			totalrate+=rate;
			pathrate.insert(std::make_pair(path_id,rate));
			}
		}
		std::map<uint32_t,uint32_t>path_weight;
		{
			uint32_t weight=0;
			for(auto it=pathrate.begin();it!=pathrate.end();it++){
				float temp_weight=(float)(packet_num*it->second)/totalrate;
				weight=(uint32_t)(temp_weight+0.5);
				path_weight.insert(std::make_pair(it->first,weight));
			}
		}
		WeightRoundRobin wrr;
		{
			for(auto it=path_weight.begin();it!=path_weight.end();it++){
				wrr.AddServer(it->first,it->second);
			}
		}
		for(auto it=packets.begin();it!=packets.end();it++){
			int server=wrr.GetServer();
            NS_ASSERT(server>0);
			sender_->PacketSchedule(it->first,(uint8_t)server);
		}
	}
}
void IdealSchedule::RetransPackets(uint32_t packet_id,
		uint8_t origin_pid){
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
void IdealSchedule::SendAllPackesToPath(std::map<uint32_t,uint32_t>&packets,
		uint8_t pid){
	for(auto it=packets.begin();it!=packets.end();it++){
		uint32_t packet_id=it->first;
		sender_->PacketSchedule(packet_id,pid);
	}
}
}


