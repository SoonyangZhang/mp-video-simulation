#include <math.h>
#include "edcldschedule.h"
#include "pathsender.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "wrr.h"
#include <string>
namespace ns3{
NS_LOG_COMPONENT_DEFINE ("EDCLDSchedule");
EDCLDSchedule::EDCLDSchedule(float weight){
	w_=(double)weight;
	last_ts_=0;
	average_packet_len_=0;
	packet_counter_=0;
}
void EDCLDSchedule::IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size){
	uint32_t n=packets.size();
    {
       // auto it=packets.begin();
       // average_packet_len_=it->second;
    }
	if(n>0&&size>0){
		average_packet_len_=(average_packet_len_*packet_counter_+size)/(packet_counter_+n);
		packet_counter_+=n;
	}
	if(pids_.size()==1||n==1){
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
			double cost=0;
			Ptr<PathSender> pathsender=sender_->GetPathInfo(temp_pid);
			cost=pathsender->GetMinRtt();
			cost_table_.insert(std::make_pair(temp_pid,cost));
			double ratio=(double)rate/totalrate;
			psi_table_.insert(std::make_pair(temp_pid,ratio));
		}
		WeightRoundRobinSchedule(packets,psi_table_);
		last_ts_=now;
	}else{
		UpdateCostAndPsiTable(now,size);
		WeightRoundRobinSchedule(packets,psi_table_);
        last_ts_=now;
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
void EDCLDSchedule::WeightRoundRobinSchedule(std::map<uint32_t,uint32_t>&packets,std::map<uint8_t,double>&lambda){
	uint32_t packet_num=packets.size();
	std::map<uint32_t,uint32_t>path_packets;
	{
		uint32_t packets=0;
		for(auto it=lambda.begin();it!=lambda.end();it++){
			if(it->second>0){
				double temp_packets=(double)(packet_num)*it->second;
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
	std::map<uint8_t,double> S_table;
	double lambda=0;
	// best is the path with the minimal cost
	{
		for(auto it=cost_table_.begin();it!=cost_table_.end();it++){
			uint8_t temp_pid=it->first;
			double updata_cost=0;
			double temp_psi=0;
			uint32_t D=0;
			Ptr<PathSender> pathsender=sender_->GetPathInfo(temp_pid);
			D=pathsender->GetMinRtt()/2;
			temp_psi=psi_table_.find(temp_pid)->second;
			uint32_t B=pathsender->GetIntantRate();
			uint32_t q=pathsender->GetPendingLen();
			// S  in bit
            double rate=0;
            if(T==0){
            
            }else{
            rate=(double)len*8*1000/T;
            if(smooth_rate_==0){
                smooth_rate_=rate;
            }else{
                smooth_rate_=(1-coeff_)*smooth_rate_+coeff_*rate;
            }
            }
			lambda=smooth_rate_;
			double S=(double)B-temp_psi*lambda;
			S_table.insert(std::make_pair(temp_pid,S));
			//q*8*1000/B  unit ms
			updata_cost=(double)D+(1-w_)*T*average_packet_len_*8/S+w_*q*8*1000/B;
			it->second=updata_cost;
		}
	}
	/* bugs, The map is sorted by key,not by value,
	auto best_it=cost_table_.begin();
	auto worst_it=cost_table_.rbegin();
	uint8_t best_pid=best_it->first;
	uint32_t worst_pid=worst_it->first;
	so change it.
	*/
	uint8_t best_pid=0;
	uint8_t worst_pid=0;
    double min_cost=0;
    double max_cost=0;
	{
		auto it=cost_table_.begin();
		uint8_t min_cost_pid=it->first;
		min_cost=it->second;
		uint8_t max_cost_pid=min_cost_pid;
		max_cost=min_cost;
		it++;
		for(;it!=cost_table_.end();it++){
			uint8_t temp_pid=it->first;
			double temp_cost=it->second;
			if(temp_cost>max_cost){
				max_cost=temp_cost;
				max_cost_pid=temp_pid;
			}
			if(temp_cost<min_cost){
				min_cost=temp_cost;
				min_cost_pid=temp_pid;
			}
		}
		best_pid=min_cost_pid;
		worst_pid=max_cost_pid;
	}

	auto best_psi_it=psi_table_.find(best_pid);
	auto worst_psi_it=psi_table_.find(worst_pid);
	double D_best=0;
	double D_worst=0;
	double B_best=0;
	double B_worst=0;
	double Q_best=0;
	double Q_worst=0;
	double S_best=0;
	double S_worst=0;
	double delta_psi=0;
/*     C_best(\psi+\Delta_{\psi})=C_worst(\psi-\Delta_{\psi})
 * 	   C unit ms
 *     L is the packet unit in M/M/1 queue
 *     L multiply  8 in bit
 *
 *       D_best-D_worst     w      q_best   q_worst
 *   k=----------------- +-------(------ - --------)
 *         (1-w)L         (1-w)L   B_best   B_worst
 *
 *   q should multiply 1000*8
 *   S=(B-\lambda*\psi)
 *   x=\Delta_{\psi}
 *              1                    1
 *   k=------------------  -  ---------------
 *      S_worst+x*\lambda       S_best-x*\lambda
 *
 *   a*x^2+b*x+c=0  .....(1) solve it.
 *
 *   a=-k{\lambda}^2
 *   b=\lambda*(2-k(S_worst-S_best))
 *   c=S_worst-S_best+k*S_worst*S_best
 *
 */

	Ptr<PathSender> pathsender;
	{
		pathsender=sender_->GetPathInfo(best_pid);
		D_best=(double)(pathsender->GetMinRtt()/2);
		B_best=(double)pathsender->GetIntantRate();
		Q_best=(double)pathsender->GetPendingLen()*1000*8;
		S_best=S_table.find(best_pid)->second;
	}
	{
		pathsender=sender_->GetPathInfo(worst_pid);
		D_worst=(double)(pathsender->GetMinRtt()/2);
		B_worst=(double)pathsender->GetIntantRate();
		Q_worst=(double)pathsender->GetPendingLen()*1000*8;
		S_worst=S_table.find(worst_pid)->second;
	}
	double L=average_packet_len_*8;//in bit
	double k=(D_best-D_worst)/((1-w_)*L)+((w_)/((1-w_)*L))*(Q_best/B_best-Q_worst/B_worst);
	double a=-k*lambda*lambda;
	double s_s=S_worst-S_best;
	double b=lambda-(2-k*s_s);
	double c=s_s+k*S_worst*S_best;
	double Delta=b*b-4*a*c;


	if(Delta<0){
		//ResetCostAndPsiTable();
        delta_psi=worst_psi_it->second;
	    double best_psi_check=best_psi_it->second+delta_psi;
	    double worst_psi_check=worst_psi_it->second-delta_psi;
        //NS_LOG_INFO(std::to_string(best_psi_it->second)<<" "<<std::to_string(worst_psi_it->second)<<" "<<std::to_string(delta_psi));
	    NS_ASSERT(best_psi_check>=0);
	    NS_ASSERT(worst_psi_check>=0);
	    best_psi_it->second=best_psi_check;
	    worst_psi_it->second=worst_psi_check;        
		//NS_LOG_WARN("no root"<<std::to_string(Delta));
		return;
	}
    double root=0;
    if(a==0){
        root=(-c)/b;
        NS_LOG_INFO("b "<<std::to_string(b));
        NS_LOG_INFO("= "<<std::to_string(root));
    }else{
        root=(-b+sqrt(Delta))/(2*a);
        NS_LOG_INFO("!= "<<std::to_string(root)<<" "<<std::to_string(Delta)<<std::to_string(lambda));
    }
	
	if(root<0){
		root=0;
		NS_LOG_WARN("root le 0");
	}
	double temp_worst_psi=worst_psi_it->second;
	double temp_psi=root;
	delta_psi=temp_psi>temp_worst_psi?temp_worst_psi:temp_psi;
	double best_psi_check=best_psi_it->second+delta_psi;
	double worst_psi_check=worst_psi_it->second-delta_psi;
    NS_LOG_INFO(std::to_string(best_psi_it->second)<<" "<<std::to_string(worst_psi_it->second)<<" "<<std::to_string(delta_psi));
	NS_ASSERT(best_psi_check>=0);
	NS_ASSERT(worst_psi_check>=0);
	best_psi_it->second=best_psi_check;
	worst_psi_it->second=worst_psi_check;
    
}
void EDCLDSchedule::ResetCostAndPsiTable(){
	std::map<uint8_t,uint32_t> pathrate;
	uint32_t totalrate=0;
	// when the key exist, the map.insert() operation will not work;
	//delete it first or map[key]=value;
	while(!cost_table_.empty()){
		auto it=cost_table_.begin();
		cost_table_.erase(it);
	}
	GetUsablePath(pathrate,totalrate);
	for(auto it=pathrate.begin();it!=pathrate.end();it++){
		uint8_t temp_pid=it->first;
		uint32_t rate=it->second;
		double cost=0;
		Ptr<PathSender> pathsender=sender_->GetPathInfo(temp_pid);
		cost=pathsender->GetMinRtt();
		cost_table_.insert(std::make_pair(temp_pid,cost));
		double ratio=(double)rate/totalrate;
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

