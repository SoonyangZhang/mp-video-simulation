#ifndef NS3_MPVIDEO_IDEALSCHEDULE_H_
#define NS3_MPVIDEO_IDEALSCHEDULE_H_
#include "interface_v1.h"
namespace zsy{
class IdealSchedule:public ScheduleV1{
public:
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,
			uint32_t size) override;
};
}
#endif /* NS3_MPVIDEO_IDEALSCHEDULE_H_ */
