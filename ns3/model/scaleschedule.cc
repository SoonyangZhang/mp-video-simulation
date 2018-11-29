#include "scaleschedule.h"
#include "pathsender.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <map>
#include <vector>
namespace ns3{
NS_LOG_COMPONENT_DEFINE("ScaleSchedule");
struct Key{
uint8_t pid;
uint32_t rtt;
};
bool Key_Comparator(const Key&a,const Key&b){
	return a.rtt<b.rtt;
}
void ScaleSchedule::IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size){
	uint32_t totalrate=0;
	uint32_t packet_num=packets.size();
	std::map<uint8_t,uint32_t> pathrate;
	if(pids_.size()==1||packet_num==1){
		RoundRobin(packets);
		return;
	}
	std::vector<Key> delay_path;
	for(auto it=pids_.begin();it!=pids_.end();it++){
		uint8_t pid=(*it);
		Ptr<PathSender> path=sender_->GetPathInfo(pid);
        uint32_t rate=0;
        uint32_t rtt=path->rtt_;
		if(cost_type_==CostType::c_intant){
			rate=path->GetIntantRate();
		}else{
			rate=path->GetSmoothRate();
		}
		if(rate!=0){
			Key key;
			key.pid=pid;
			key.rtt=rtt;
			totalrate+=rate;
			delay_path.push_back(key);
			pathrate.insert(std::make_pair(pid,rate));
		}
	}
	std::sort(delay_path.begin(),delay_path.end(),Key_Comparator);
	int useable_path=pathrate.size();
	int i=0;
	std::map<uint8_t,uint32_t> pathpacket;
	uint32_t remain=packet_num;
	{
		auto it=pathrate.begin();
		double rate=(double)it->second;
		for(i=0;i<useable_path;i++){
			uint32_t num=(uint32_t)(packet_num*rate/totalrate+0.5);
			uint8_t pid=it->first;
			if((i+1)==useable_path){
				if(remain>0){
					pathpacket.insert(std::make_pair(pid,remain));
				}
			    break;
			}else{
				if(remain>0){
					pathpacket.insert(std::make_pair(pid,num));
				}
			}
			it++;
			remain-=num;
		}
	}
	if(totalrate==0){
		RoundRobin(packets);
	}else{
		auto it=packets.begin();
		for(auto delay_path_it=delay_path.begin();delay_path_it!=delay_path.end();delay_path_it++){
			int j=0;
			uint8_t temp_pid=delay_path_it->pid;
			auto path_it=pathpacket.find(temp_pid);
			uint32_t path_i_num=path_it->second;
			for(j=0;j<path_i_num;j++){
				sender_->PacketSchedule(it->first,temp_pid);
				it++;
			}
		}
		if(it!=packets.end()){
			NS_LOG_ERROR("fatal error");
		}
	}
}
void ScaleSchedule::RoundRobin(std::map<uint32_t,uint32_t>&packets){
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
void ScaleSchedule::RetransPackets(std::map<uint32_t,uint32_t>&packets){

}
}



