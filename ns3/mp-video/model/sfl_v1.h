#ifndef NS3_MPVIDEO_SFL_V1_H_
#define NS3_MPVIDEO_SFL_V1_H_
#include "interface_v1.h"
namespace zsy{
typedef struct{
	uint8_t pid;
	uint32_t owd;
	uint32_t bps;
	int32_t byte;
}path_water_t;
class SFLScheduleV1:public ScheduleV1{
public:
	SFLScheduleV1(){}
	~SFLScheduleV1(){}
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,
			uint32_t size) override;
private:
	void AllocateWater(std::vector<path_water_t> &watertable,uint32_t filling,uint32_t step);
	uint32_t GetTotalWater(std::vector<path_water_t> &watertable,uint32_t compensate);
};
}




#endif /* NS3_MPVIDEO_SFL_V1_H_ */
