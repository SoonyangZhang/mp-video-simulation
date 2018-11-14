#ifndef NS3_MPVIDEO_FUN_TEST_T_INTERFACE_H_
#define NS3_MPVIDEO_FUN_TEST_T_INTERFACE_H_
#include "ns3/mpcommon.h"
namespace ns3{
class DataTarget{
public:
	virtual ~DataTarget(){}
	virtual void OnFrame(uint8_t*data,uint32_t len)=0;
};
class PacketSentObserver{
public:
	virtual ~PacketSentObserver(){}
	virtual void OnSent(uint32_t len)=0;
};
}
#endif /* NS3_MPVIDEO_FUN_TEST_T_INTERFACE_H_ */
