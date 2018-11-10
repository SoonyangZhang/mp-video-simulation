#include "wrrschedule.h"
#include "wrr.h"
#include "pathsender.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <map>
#include<string>
namespace ns3{
NS_LOG_COMPONENT_DEFINE("WrrSchedule");
void WrrSchedule::IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size){
	uint32_t totalrate=0;
	uint32_t packet_num=packets.size();
	std::map<uint8_t,uint32_t> pathrate;
	if(pids_.size()==1){
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
	std::map<uint32_t,uint32_t>path_weight;
    std::map<uint32_t,uint32_t> record;
	{
		uint32_t weight=0;
		for(auto it=pathrate.begin();it!=pathrate.end();it++){
			float temp_weight=(float)(packet_num*it->second)/totalrate;
			weight=(uint32_t)(temp_weight+0.5);
			path_weight.insert(std::make_pair(it->first,weight));
            record.insert(std::make_pair(it->first,0));
		}
	}
	WeightRoundRobin wrr;
	{
		for(auto it=path_weight.begin();it!=path_weight.end();it++){
			wrr.AddServer(it->first,it->second);
		}
	}
	if(totalrate==0){
		RoundRobin(packets);
	}else{
		for(auto it=packets.begin();it!=packets.end();it++){
			int server=wrr.GetServer();
            NS_ASSERT(server>0);
            auto recordit=record.find(server);            
            recordit->second++;
			sender_->PacketSchedule(it->first,(uint8_t)server);
		}
        for(auto recordit=record.begin();recordit!=record.end();recordit++)
        {
            //NS_LOG_INFO(std::to_string(recordit->first)<<" "<<std::to_string(recordit->second));
        }
	}
}
void WrrSchedule::RoundRobin(std::map<uint32_t,uint32_t>&packets){
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
void WrrSchedule::RetransPackets(std::map<uint32_t,uint32_t>&packets){

}
}
