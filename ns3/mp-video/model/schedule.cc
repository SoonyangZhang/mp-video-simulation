#include "schedule.h"
namespace ns3{
void Schedule::RegisterPath(uint8_t pid){
	for(auto it=pids_.begin();it!=pids_.end();it++){
		if((*it)==pid){
			return;
		}
	}
	pids_.push_back(pid);
}
void Schedule::UnregisterPath(uint8_t pid){
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




