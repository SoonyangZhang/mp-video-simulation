#include "balancecostschedule.h"
#include "pathsender.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <map>
#include<string>
namespace ns3{
NS_LOG_COMPONENT_DEFINE("BalanceCostSchedule");
void BalanceCostSchedule::IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size){
	uint32_t totalrate=0;
	uint32_t packet_num=packets.size();
	std::map<uint8_t,uint32_t> pathrate;
	if(pids_.size()==1||packet_num==1){
		RoundRobin(packets);
		return;
	}

	for(auto it=pids_.begin();it!=pids_.end();it++){
		uint8_t pid=(*it);
		Ptr<PathSender> path=sender_->GetPathInfo(pid);
        uint32_t rate=0;
		if(cost_type_==CostType::c_intant){
			rate=path->GetIntantRate();
		}else{
			rate=path->GetSmoothRate();
		}
		if(rate!=0){			
			totalrate+=rate;
			pathrate.insert(std::make_pair(pid,rate));
		}
	}
	for(auto it=packets.begin();it!=packets.end();it++){
		auto path_rate_it=pathrate.begin();
		uint8_t min_pid=path_rate_it->first;
		Ptr<PathSender> path=sender_->GetPathInfo(min_pid);
		uint32_t min_cost=path->GetCost();
		path_rate_it++;
		for(;path_rate_it!=pathrate.end();path_rate_it++){
			uint8_t pathid=path_rate_it->first;
			path=sender_->GetPathInfo(pathid);
			uint32_t temp_cost=0;
			if(cost_type_==CostType::c_intant){
				temp_cost=path->GetCost();
			}else{
				temp_cost=path->GetSmoothCost();
			}
			if(temp_cost<min_cost){
				min_cost=temp_cost;
				min_pid=pathid;
			}
		}
		sender_->PacketSchedule(it->first,min_pid);
	}
}
void BalanceCostSchedule::RoundRobin(std::map<uint32_t,uint32_t>&packets){
	uint8_t total=pids_.size();
    uint32_t last_index=0;
	for(auto it=packets.begin();it!=packets.end();it++){
		uint32_t packet_id=it->first;
        uint32_t index=last_index%total;
        last_index++;
		uint8_t pid=pids_[index];
		sender_->PacketSchedule(packet_id,pid);
	}
}
void BalanceCostSchedule::RetransPackets(std::map<uint32_t,uint32_t>&packets){

}
}



