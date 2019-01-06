#ifndef NS3_MPVIDEO_SCALESCHEDULE_H_
#define NS3_MPVIDEO_SCALESCHEDULE_H_
#include "schedule.h"
namespace ns3{
class ScaleSchedule:public Schedule{
public:
	ScaleSchedule(uint8_t type){cost_type_=type;}
	~ScaleSchedule(){}
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size) override;
	void RetransPackets(std::map<uint32_t,uint32_t>&packets) override;
	void RoundRobin(std::map<uint32_t,uint32_t>&packets);
private:
	uint8_t cost_type_;
};
}




#endif /* NS3_MPVIDEO_SCALESCHEDULE_H_ */
