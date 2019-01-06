#include "ns3/bc_v1.h"
#include "ns3/log.h"
#include "ns3/path_sender_v1.h"
#include <algorithm>
#include <iostream>
using namespace ns3;
namespace zsy{
NS_LOG_COMPONENT_DEFINE("BCScheduleV1");
void BCScheduleV1::IncomingPackets(std::map<uint32_t,uint32_t>&packets,
		uint32_t size){
	   uint32_t packet_num=packets.size();
		if(pids_.size()==1){
			uint8_t path_id=pids_[0];
			SendAllPackesToPath(packets,path_id);
			return;
		}
		if(packet_num==1){
			auto pid_it=pids_.begin();
			uint8_t min_rtt_pid=(*pid_it);
			Ptr<PathSenderV1> subpath=sender_->GetPathInfo(min_rtt_pid);
			uint32_t min_rtt=subpath->get_rtt();
			pid_it++;
			for(;pid_it!=pids_.end();pid_it++){
				uint8_t pid=(*pid_it);
				subpath=sender_->GetPathInfo(pid);
				uint32_t rtt=subpath->get_rtt();
				if(rtt<min_rtt){
					min_rtt=rtt;
					min_rtt_pid=pid;
				}
			}
			SendAllPackesToPath(packets,min_rtt_pid);
			return;
		}
	    std::map<uint8_t,uint32_t> pathrate;
        uint32_t totalrate=0;
		for(auto it=pids_.begin();it!=pids_.end();it++){
			uint8_t pid=(*it);
			Ptr<PathSenderV1> subpath=sender_->GetPathInfo(pid);
	        uint32_t rate=subpath->GetIntantRate();
			if(rate!=0){
				totalrate+=rate;
				pathrate.insert(std::make_pair(pid,rate));
			}
		}
		for(auto it=packets.begin();it!=packets.end();it++){
			auto path_rate_it=pathrate.begin();
			uint8_t min_pid=path_rate_it->first;
			uint32_t bps=path_rate_it->second;
			Ptr<PathSenderV1> subpath=sender_->GetPathInfo(min_pid);
			uint32_t min_cost=subpath->get_cost();
			path_rate_it++;
			for(;path_rate_it!=pathrate.end();path_rate_it++){
				uint8_t pathid=path_rate_it->first;
				subpath=sender_->GetPathInfo(pathid);
				uint32_t temp_cost=subpath->get_cost();
				if(temp_cost<min_cost){
					min_cost=temp_cost;
					min_pid=pathid;
				}
			}
			sender_->PacketSchedule(it->first,min_pid);
		}
}
}



