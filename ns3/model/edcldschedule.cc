#include <math.h>
#include "edcldschedule.h"
#include "pathsender.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "wrr.h"
namespace ns3{
EDCLDSchedule::EDCLDSchedule(float weight){
	w_=weight;
	last_ts_=0;
	average_packet_len_=0;
	packet_counter_=0;
}
void EDCLDSchedule::IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size){
	uint32_t n=packets.size();
	if(n>0&&size>0){
		average_packet_len_=(average_packet_len_*packet_counter_+size)/(packet_counter_+n);
		packet_counter_+=n;
	}
	if(pids_.size()==1){
		RoundRobin(packets);
		return;
	}
	uint32_t now=Simulator::Now().GetMilliSeconds();
	if(last_ts_==0){
		std::map<uint8_t,uint32_t> pathrate;
		uint32_t totalrate=0;
		GetUsablePath(pathrate,totalrate);
		for(auto it=pathrate.begin();it!=pathrate.end();it++){
			uint8_t temp_pid=it->first;
			uint32_t rate=it->second;
			float cost=0;
			Ptr<PathSender> pathsender=sender_->GetPathInfo(temp_pid);
			cost=pathsender->GetMinRtt();
			cost_table_.insert(std::make_pair(temp_pid,cost));
			float ratio=(float)rate/totalrate;
			psi_table_.insert(std::make_pair(temp_pid,ratio));
		}
		WeightRoundRobinSchedule(packets,psi_table_);
		last_ts_=now;
	}else{
		UpdateCostAndPsiTable(now,size);
		WeightRoundRobinSchedule(packets,psi_table_);
	}
}
void EDCLDSchedule::GetUsablePath(std::map<uint8_t,uint32_t>&usable_path,uint32_t &total_rate){
	for(auto it=pids_.begin();it!=pids_.end();it++){
		uint8_t pid=(*it);
		Ptr<PathSender> path=sender_->GetPathInfo(pid);
		uint32_t rate=path->GetIntantRate();
		if(rate!=0){
			total_rate+=rate;
			usable_path.insert(std::make_pair(pid,rate));
		}
	}
}
void EDCLDSchedule::RoundRobin(std::map<uint32_t,uint32_t>&packets){
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
void EDCLDSchedule::WeightRoundRobinSchedule(std::map<uint32_t,uint32_t>&packets,std::map<uint8_t,float>&lambda){
	uint32_t packet_num=packets.size();
	std::map<uint32_t,uint32_t>path_packets;
	{
		uint32_t packets=0;
		for(auto it=lambda.begin();it!=lambda.end();it++){
			if(it->second>0){
				float temp_packets=(float)(packet_num)*it->second;
				packets=(uint32_t)temp_packets;
				if(packets>0){
					path_packets.insert(std::make_pair(it->first,packets));
				}
			}
		}
	}
	WeightRoundRobin wrr;
	{
		for(auto it=path_packets.begin();it!=path_packets.end();it++){
			wrr.AddServer(it->first,it->second);
		}
	}
	for(auto it=packets.begin();it!=packets.end();it++){
		int server=wrr.GetServer();
        NS_ASSERT(server>0);
		sender_->PacketSchedule(it->first,(uint8_t)server);
	}
}
void EDCLDSchedule::RetransPackets(std::map<uint32_t,uint32_t>&packets){

}
void EDCLDSchedule::UpdateCostAndPsiTable(uint32_t now,uint32_t len){
	uint32_t T=now-last_ts_;
	std::map<uint8_t,float> S_table;
	// best is the path with the minimal cost
	{
		for(auto it=cost_table_.begin();it!=cost_table_.end();it++){
			uint8_t temp_pid=it->first;
			float updata_cost=0;
			float temp_psi=0;
			uint32_t D=0;
			Ptr<PathSender> pathsender=sender_->GetPathInfo(temp_pid);
			D=pathsender->GetMinRtt();
			temp_psi=psi_table_.find(temp_pid)->second;
			uint32_t B=pathsender->GetIntantRate();
			uint32_t q=pathsender->GetPendingLen();
			// S  in bit
			float S=(float)(B*T/1000-len*temp_psi*8);
			S_table.insert(std::make_pair(temp_pid,S));
			//q*8*1000/B  unit ms
			updata_cost=(float)D+(1-w_)*T*average_packet_len_*8/S+w_*q*8*1000/B;
			it->second=updata_cost;
		}
	}
	auto best_it=cost_table_.begin();
	auto worst_it=cost_table_.rbegin();
	uint8_t best_pid=best_it->first;
	uint32_t worst_pid=worst_it->first;
	auto best_psi_it=psi_table_.find(best_pid);
	auto worst_psi_it=psi_table_.find(worst_pid);
	uint32_t D_best=0;
	uint32_t D_worst=0;
	float S_best=0; // 	Multiply T
	float S_worst=0;
	int v=0;
	float delta_psi=0;
/*S=(B-\lambda*\psi)T
 *
 *             (S_best-S_worst)T
 *     (1)     ------------------   D_best==D_worst
 *			   \lambda*T=len
 *
 *delta_psi=
 *            (S_best-S_worst)T+2*T/delta{D_p}-vT*sqrt((S_best+S_worst)^2+(2/delta{D_p})^2)
 *            -------------------------------------------------------------
 *     (2)                   2*\lambda*T
 *                                            otherwise
 */
	Ptr<PathSender> pathsender;
	{
		pathsender=sender_->GetPathInfo(best_pid);
		D_best=pathsender->GetMinRtt();
		S_best=S_table.find(best_pid)->second;
	}
	{
		pathsender=sender_->GetPathInfo(worst_pid);
		D_worst=pathsender->GetMinRtt();
		S_worst=S_table.find(worst_pid)->second;
	}
	if(D_best==D_worst){
		float temp_psi=0;
		float temp_worst_psi=worst_psi_it->second;
		temp_psi=(S_best-S_worst)/(float)(2*len*8);
		delta_psi=temp_psi>temp_worst_psi?temp_worst_psi:temp_psi;
	}else{
		if(D_best>D_worst){
			v=1;
		}else{
			v=-1;
		}
		float temp_psi=0;
		float second_term=float(2*T)/(D_best-D_worst)*average_packet_len_*8;
		float temp_worst_psi=worst_psi_it->second;
		float sxs=(S_best+S_worst)*(S_best+S_worst);
		temp_psi=(S_best-S_worst+second_term
				-v*sqrt(sxs+second_term*second_term))/(float)(2*len*8);
		delta_psi=temp_psi>temp_worst_psi?temp_worst_psi:temp_psi;
	}
	float best_psi_check=best_psi_it->second+delta_psi;
	float worst_psi_check=worst_psi_it->second-delta_psi;
	NS_ASSERT(best_psi_check>=0);
	NS_ASSERT(worst_psi_check>=0);
	best_psi_it->second=best_psi_check;
	worst_psi_it->second=worst_psi_check;
	last_ts_=now;
}
void EDCLDSchedule::ResetCostAndPsiTable(){
	std::map<uint8_t,uint32_t> pathrate;
	uint32_t totalrate=0;
	GetUsablePath(pathrate,totalrate);
	for(auto it=pathrate.begin();it!=pathrate.end();it++){
		uint8_t temp_pid=it->first;
		uint32_t rate=it->second;
		float cost=0;
		Ptr<PathSender> pathsender=sender_->GetPathInfo(temp_pid);
		cost=pathsender->GetMinRtt();
		cost_table_.insert(std::make_pair(temp_pid,cost));
		float ratio=(float)rate/totalrate;
		psi_table_.insert(std::make_pair(temp_pid,ratio));

}
}
void EDCLDSchedule::RegisterPath(uint8_t pid){
	Schedule::RegisterPath(pid);
	if(last_ts_!=0){
		ResetCostAndPsiTable();
	}
}
void EDCLDSchedule::UnregisterPath(uint8_t pid){
	Schedule::UnregisterPath(pid);
	ResetCostAndPsiTable();
}
}

