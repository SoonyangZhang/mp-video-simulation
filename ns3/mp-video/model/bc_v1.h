#ifndef NS3_MPVIDEO_BC_V1_H_
#define NS3_MPVIDEO_BC_V1_H_
#include "interface_v1.h"
namespace zsy{
class BCScheduleV1:public ScheduleV1{
public:
	BCScheduleV1(){}
	~BCScheduleV1(){}
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,
			uint32_t size) override;
};
}
#endif /* NS3_MPVIDEO_BC_V1_H_ */
