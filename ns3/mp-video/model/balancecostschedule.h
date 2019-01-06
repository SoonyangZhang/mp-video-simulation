#ifndef NS3_MPVIDEO_BALANCECOSTSCHEDULE_H_
#define NS3_MPVIDEO_BALANCECOSTSCHEDULE_H_
#include "schedule.h"
namespace ns3{
class BalanceCostSchedule:public Schedule{
public:
	BalanceCostSchedule(uint8_t type){cost_type_=type;}
	~BalanceCostSchedule(){}
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size) override;
	void RetransPackets(std::map<uint32_t,uint32_t>&packets) override;
	void RoundRobin(std::map<uint32_t,uint32_t>&packets);
private:
	uint8_t cost_type_;
};
}
#endif /* NS3_MPVIDEO_BALANCECOSTSCHEDULE_H_ */
