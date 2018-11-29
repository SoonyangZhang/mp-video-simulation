/* The code implementation of the paper
 * Effective Delay-Controlled Load Distribution over Multipath Networks
 *                                1                q_p
 * C_p(\psi_p) =D_p+(1-w)-----------------   +  w-------
 *                           u_p-\psi_p*\lambda    u_p
 * 1  should have some unit, one person and here I use average packet size;
 * https://en.wikipedia.org/wiki/M/M/1_queue
 * D_p propagation delay
 * u_p service rate or the packet outgoing rate
 * \lambda the packet generate rate from the source
 * q_p the pending queue length
 * To make the problem simple, I make minor change to the second
 * term of the origin equation:
 *                T
 * (1-w)-----------------------
 *      (u_p-\psi_p*\lambda)*T
 * here, T is the schedule duration or the frame generating rate.
 * \lambda*T=Length
 * The Optimization is min max C_p(\psi_p)
 * The solution of in appendix of the paper can be found in another paper of the same auther,
 * "Load Distribution with Queuing Delay Bound over Multipath Networks:
 * Rate Control Using Stochastic Delay Prediction".
 */
#ifndef NS3_MPVIDEO_EDCLDSCHEDULE_H_
#define NS3_MPVIDEO_EDCLDSCHEDULE_H_
#include <map>
#include "schedule.h"
namespace ns3{
class EDCLDSchedule: public Schedule{
public:
	EDCLDSchedule(float weight);
	~EDCLDSchedule(){}
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size) override;
	void RetransPackets(std::map<uint32_t,uint32_t>&packets) override;
	void RegisterPath(uint8_t pid) override;
	void UnregisterPath(uint8_t pid) override;
private:
	void GetUsablePath(std::map<uint8_t,uint32_t>&usable_path,uint32_t &total_rate);
	void RoundRobin(std::map<uint32_t,uint32_t>&packets);
	void WeightRoundRobinSchedule(std::map<uint32_t,uint32_t>&packets,std::map<uint8_t,double>&lambda);
	void UpdateCostAndPsiTable(uint32_t now,uint32_t len);
	void ResetCostAndPsiTable();
	double w_;
	uint32_t last_ts_;
	std::map<uint8_t,double> cost_table_;
	std::map<uint8_t,double> psi_table_;
	uint32_t average_packet_len_;
	uint64_t packet_counter_;
};
}
#endif /* NS3_MPVIDEO_EDCLDSCHEDULE_H_ */
