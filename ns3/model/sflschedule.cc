#include "sflschedule.h"
#include "ns3/log.h"
#include "pathsender.h"
#include <algorithm>
namespace ns3{
NS_LOG_COMPONENT_DEFINE("SFLSchedule");
bool Water_Comparator(const path_water_t&a,const path_water_t&b){
	return a.owd<b.owd;
}
void SFLSchedule::IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size){
	if(pids_.size()==1){
		RoundRobin(packets);
		return;
	}
	std::vector<path_water_t> delay_water_table;
	for(auto it=pids_.begin();it!=pids_.end();it++){
		uint8_t pid=(*it);
		Ptr<PathSender> path=sender_->GetPathInfo(pid);
		path_water_t water;
		water.pid=pid;
		water.owd=path->rtt_/2;
		water.bps=path->GetIntantRate();
		water.byte=0;
		if(water.bps>0){
			delay_water_table.push_back(water);
		}
	}
	std::sort(delay_water_table.begin(),delay_water_table.end(),Water_Comparator);
    uint32_t step=0;
    {
        auto it=packets.begin();
        uint32_t len=it->second;
        auto water_it=delay_water_table.rbegin();
        uint32_t bps=water_it->bps;
        double ms=(double)len*8*1000/bps+0.5;
        step=(uint32_t)ms;
        //when the water is not enough, the slowest path incrase at step at least one packet;
    }
	AllocateWater(delay_water_table,size,step);
	for(auto it=packets.begin();it!=packets.end();it++){
		while(!delay_water_table.empty()){
			auto water_it=delay_water_table.begin();
			uint8_t pid=water_it->pid;
			if(delay_water_table.size()==1){
				sender_->PacketSchedule(it->first,pid);
				break;
			}else{
				int last=water_it->byte;
				if(last>0/*&&last>=it->second*/){
                    sender_->PacketSchedule(it->first,pid);
                    int remain=last-it->second;
                    if(remain>0)
                    {
                        water_it->byte=remain;
                    }else{
                        water_it->byte=0;
                    }
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
void SFLSchedule::AllocateWater(std::vector<path_water_t> &watertable,uint32_t filling,uint32_t step){
	int i=0;
    uint32_t compensate=0;
	for(i=0;;i++){
		compensate=step*i;
		uint32_t water=GetTotalWater(watertable,compensate);
		if(water>=filling){
			break;
		}
	}
}
uint32_t SFLSchedule::GetTotalWater(std::vector<path_water_t> &watertable,uint32_t compensate){
	uint32_t water=0;
	int depth=0;
	{
		auto it=watertable.rbegin();
		depth=it->owd+compensate;
	}
	for(auto it=watertable.begin();it!=watertable.end();it++){
		path_water_t *temp_info=&(*it);
		int temp=(depth-temp_info->owd)*temp_info->bps/(8*1000);
		NS_ASSERT(temp>=0);
		it->byte=temp;
		water+=temp;
	}
	return water;
}
}




