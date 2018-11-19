#include "sflschedule.h"
#include "ns3/log.h"
#include "pathsender.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE("SFLSchedule");
void SFLSchedule::IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size){
	uint32_t totalrate=0;
	uint32_t packet_num=packets.size();
	std::map<uint8_t,uint32_t> pathrate;
	if(pids_.size()==1){
		RoundRobin(packets);
		return;
	}
	std::map<uint32_t,path_water_t> delay_water_table;
	for(auto it=pids_.begin();it!=pids_.end();it++){
		uint8_t pid=(*it);
		Ptr<PathSender> path=sender_->GetPathInfo(pid);
		path_water_t water;
		water.pid=pid;
		water.owd=path->rtt_/2;
		water.bps=path->GetSmoothRate();
		water.byte=0;
		water.number=0;
		if(water.bps>0){
			delay_water_table.insert(std::make_pair(water.owd,water));
		}
	}
	AllocateWater(delay_water_table,size);
	for(auto it=packets.begin();it!=packets.end();it++){
		while(!delay_water_table.empty()){
			auto water_it=delay_water_table.begin();
			uint8_t pid=water_it->second.pid;
			if(delay_water_table.size()==1){
				sender_->PacketSchedule(it->first,pid);
				break;
			}else{
				int remain=water_it->second.byte;
				if(remain>=it->second){
					water_it->second.byte=remain-it->second;
					sender_->PacketSchedule(it->first,pid);
					break;
				}else{
					delay_water_table.erase(water_it);
				}
			}
		}
	}
}
void SFLSchedule::RetransPackets(std::map<uint32_t,uint32_t>&packets){

}
void SFLSchedule::RoundRobin(std::map<uint32_t,uint32_t>&packets){
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
void SFLSchedule::AllocateWater(std::map<uint32_t,path_water_t> &watertable,uint32_t filling){
	uint32_t step=5;// water depth increase;
	uint32_t compensate=0;
	int i=0;
	for(i=0;;i++){
		compensate=step*i;
		uint32_t water=GetTotalWater(watertable,compensate);
		if(water>=filling){
			break;
		}
	}
}
uint32_t SFLSchedule::GetTotalWater(std::map<uint32_t,path_water_t> &watertable,uint32_t compensate){
	uint32_t water=0;
	int depth=0;
	{
		auto it=watertable.rbegin();
		depth=it->second.owd+compensate;
	}
	for(auto it=watertable.begin();it!=watertable.end();it++){
		path_water_t *temp_info;
		temp_info=&(it->second);
		int temp=(depth-temp_info->owd)*temp_info->bps/(8*1000);
		NS_ASSERT(temp>=0);
		it->second.byte=temp;
		water+=temp;
	}
	return water;
}
}




