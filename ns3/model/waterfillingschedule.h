/*
 * an optimized version of water filling algorithm
 */
#ifndef NS3_MPVIDEO_WATERFILLINGSCHEDULE_H_
#define NS3_MPVIDEO_WATERFILLINGSCHEDULE_H_
#include "schedule.h"
#include "sflschedule.h"
namespace ns3{
class WaterFillingSchedule: public Schedule{
public:
	WaterFillingSchedule(){}
	~WaterFillingSchedule(){}
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size) override;
	void RetransPackets(std::map<uint32_t,uint32_t>&packets) override;
	void RoundRobin(std::map<uint32_t,uint32_t>&packets);
private:
	void AllocateWater(std::vector<path_water_t> &watertable,uint32_t filling,uint32_t step);
	uint32_t GetTotalWater(std::vector<path_water_t> &watertable,uint32_t compensate);

};
}
#endif /* NS3_MPVIDEO_WATERFILLINGSCHEDULE_H_ */
