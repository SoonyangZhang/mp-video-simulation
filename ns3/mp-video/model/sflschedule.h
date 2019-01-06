/* Code implementation of paper
 * "A low-latency scheduling approach for
 * high-definition video streaming in a
 * heterogeneous wireless network with multihomed clients"
 *  packets is scheduling with the water filling algorithm
 */
#ifndef NS3_MPVIDEO_SFLSCHEDULE_H_
#define NS3_MPVIDEO_SFLSCHEDULE_H_
#include "schedule.h"
#include <vector>
namespace ns3{
typedef struct{
	uint8_t pid;
	uint32_t owd;
	uint32_t bps;
	int32_t byte;
}path_water_t;
bool Water_Comparator(const path_water_t&a,const path_water_t&b);
class SFLSchedule :public Schedule{
public:
	SFLSchedule(){}
	~SFLSchedule(){}
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size) override;
	void RetransPackets(std::map<uint32_t,uint32_t>&packets) override;
	void RoundRobin(std::map<uint32_t,uint32_t>&packets);
private:
	void AllocateWater(std::vector<path_water_t> &watertable,uint32_t filling,uint32_t step);
	uint32_t GetTotalWater(std::vector<path_water_t> &watertable,uint32_t compensate);
};
}




#endif /* NS3_MPVIDEO_SFLSCHEDULE_H_ */
