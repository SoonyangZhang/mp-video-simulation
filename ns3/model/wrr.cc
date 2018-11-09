#include "wrr.h"
namespace ns3{
void WeightRoundRobin::AddServer(int id,uint32_t weight){
	for(auto it=servers_.begin();it!=servers_.end();it++){
		if(it->id==id){
			return ;
		}
	}
	if(weight>0){
		ServerInfo info(id,weight);
		servers_.push_back(info);
	}
}
int WeightRoundRobin::GetServer(){
	int n=servers_.size();
	int gcd=GetGCD();
	max_weight_=GetMaxWeight();
	while(true){
		i_=(i_+1)%n;
		if(i_==0){
			cw_=cw_-gcd;
			if(cw_<=0){
				cw_=max_weight_;
			}
		}
		if(servers_[i_].weight>=cw_){
			return servers_[i_].id;
		}
	}
	return -1;
}
int WeightRoundRobin::GetGCD(){
	return 1;
}
uint32_t WeightRoundRobin::GetMaxWeight(){
	uint32_t max_weight=0;
	for(auto it=servers_.begin();it!=servers_.end();it++){
		uint32_t weight=it->weight;
		if(weight>max_weight){
			max_weight=weight;
		}
	}
	return max_weight;
}
}


