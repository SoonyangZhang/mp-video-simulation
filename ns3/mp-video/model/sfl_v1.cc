#include "ns3/sfl_v1.h"
#include "ns3/log.h"
#include "ns3/path_sender_v1.h"
#include <algorithm>
#include <iostream>
using namespace ns3;
namespace zsy{
NS_LOG_COMPONENT_DEFINE("SFLScheduleV1");
bool Water_Comparator(const path_water_t&a,const path_water_t&b){
	return a.owd<b.owd;
}
void SFLScheduleV1::IncomingPackets(std::map<uint32_t,uint32_t>&packets,
		uint32_t size){
	   uint32_t packet_num=packets.size();
		if(pids_.size()==1){
			uint8_t path_id=pids_[0];
			SendAllPackesToPath(packets,path_id);
			return;
		}
		std::vector<path_water_t> delay_water_table;
		for(auto it=pids_.begin();it!=pids_.end();it++){
			uint8_t pid=(*it);
			Ptr<PathSenderV1> subpath=sender_->GetPathInfo(pid);
			path_water_t water;
			water.pid=pid;
			water.owd=subpath->get_aggregate_delay();//subpath->get_cost();//subpath->get_rtt()/2;
			water.bps=subpath->GetIntantRate();
			water.byte=0;
	        if(water.bps==0)
	        {
	            NS_LOG_WARN("rate0");
	        }
			if(water.bps>0){
				delay_water_table.push_back(water);
			}
		}
		std::sort(delay_water_table.begin(),delay_water_table.end(),Water_Comparator);
	    if(packet_num==1){
	    	uint8_t min_rtt_pid=0;
	    	auto delay_water_table_it=delay_water_table.begin();
	    	min_rtt_pid=delay_water_table_it->pid;
			SendAllPackesToPath(packets,min_rtt_pid);
			return ;
	    }
		uint32_t L=0;
	    uint32_t step=0;
	    {
	        auto it=packets.begin();
	        uint32_t L=it->second;
	        auto water_it=delay_water_table.rbegin();
	        uint32_t bps=water_it->bps;
	        double ms=(double)L*8*1000/bps;
	        step=(uint32_t)ms;
	        NS_LOG_INFO(std::to_string(L)<<" "<<std::to_string(bps)<<" "<<std::to_string(step));
	        //when the water is not enough, the slowest path incrase at step at least one packet;
	    }
		AllocateWater(delay_water_table,size,step);
	    {
	        for(auto it=delay_water_table.begin();it!=delay_water_table.end();it++){
	            NS_LOG_INFO(std::to_string(it->pid)<<" "<<std::to_string(it->byte)<<" "<<std::to_string(size)<<" "<<std::to_string(it->bps));
	        }
	    }

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
	                    if(remain>=L)
	                    {
	                        water_it->byte=remain;
	                    }else{
	                        delay_water_table.erase(water_it);
	                    }
						break;
					}else{
						delay_water_table.erase(water_it);
					}
				}
			}
		}
}
void SFLScheduleV1::AllocateWater(std::vector<path_water_t> &watertable,uint32_t filling,uint32_t step){
	int i=0;
    uint32_t compensate=0;
	for(i=0;;i++){
		compensate=step*i;
		uint32_t water=GetTotalWater(watertable,compensate);
		if(water>=filling){
			break;
		}
	}
    return ;
}
uint32_t SFLScheduleV1::GetTotalWater(std::vector<path_water_t> &watertable,uint32_t compensate){
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




