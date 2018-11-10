#ifndef NS3_MPVIDEO_ROUNDROBINSCHEDULE_H_
#define NS3_MPVIDEO_ROUNDROBINSCHEDULE_H_
#include <vector>
#include "schedule.h"
namespace ns3{
// implement in round robin way
class RoundRobinSchedule:public Schedule{
public:
	RoundRobinSchedule();
	~RoundRobinSchedule(){}
void IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size) override;
void RetransPackets(std::map<uint32_t,uint32_t>&packets) override;
private:
    uint32_t last_index_;
};

}

#endif /* NS3_MPVIDEO_ROUNDROBINSCHEDULE_H_ */
