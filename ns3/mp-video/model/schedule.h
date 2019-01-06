#ifndef MULTIPATH_SCHEDULE_H_
#define MULTIPATH_SCHEDULE_H_
#include <map>
#include <vector>
#include "mpcommon.h"
namespace ns3{
enum CostType{
	c_intant,
	c_smooth,
};
class Schedule{
public:
	virtual ~Schedule(){}
	void SetSender(SenderInterface* s){sender_=s;}
	//std::map<uint32_t,uint32_t>&packets  packet id ->length
	virtual void IncomingPackets(std::map<uint32_t,uint32_t>&packets,uint32_t size)=0;
	virtual void RetransPackets(std::map<uint32_t,uint32_t>&packets)=0;
	virtual void RegisterPath(uint8_t);
	virtual void UnregisterPath(uint8_t);
protected:
    std::vector<uint8_t> pids_;
protected:
	SenderInterface *sender_;
};
}
#endif /* MULTIPATH_SCHEDULE_H_ */
