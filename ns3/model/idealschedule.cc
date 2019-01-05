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
            rate=path->get_rate();
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
}


