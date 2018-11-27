#ifndef SIM_TRANSPORT_MPCOMMON_H_
#define SIM_TRANSPORT_MPCOMMON_H_
#include "cf_platform.h"
#include "sim_proto.h"
#include <stdint.h>
#include "ns3/ptr.h"
#include "ns3/simulationclock.h"
#define WEBRTC_POSIX
#define WEBRTC_LINUX
namespace ns3{
#define MIN_BITRATE		80000					/*10KB*/
#define MAX_BITRATE		16000000				/*2MB*/
#define START_BITRATE	800000					/*100KB*/
#define SMOOTH_RATE_NUM 90
#define SMOOTH_RATE_DEN 100
#define MAX_QUEUE_TIME 1000 //ms
typedef void(*sim_notify_fn)(void* event, int type, uint32_t val);
class PathSender;
class RateControl;
class Schedule;
class VideoSource;
enum PathState{
	path_ini,
	path_conning,
	path_conned,
	path_discon,
};
enum{
	gcc_transport = 0,
	bbr_transport = 1,
};
enum NOTIFYMESSAGE{
	notify_unknow,
	notify_con_ack,
	notify_con,
    notify_dis,
	notify_dis_ack,
};
struct TraceFrameInfo{
	uint32_t fid;
	uint32_t len;
	uint32_t recv;
	uint32_t total;
	uint32_t delay;
};
//to consume frame
class NetworkDataConsumer{
public:
	virtual ~NetworkDataConsumer(){}
	virtual void ForwardUp(uint32_t fid,uint8_t*data,uint32_t len,uint32_t recv,uint32_t total)=0;
};
class ReceiverInterface{
public:
	virtual ~ReceiverInterface(){}
    virtual uint32_t GetUid()=0;
    virtual void DeliverToCache(uint8_t pid,sim_segment_t* d)=0;
    virtual void StopCallback()=0;
};
class SenderInterface{
public:
	virtual ~SenderInterface(){}
	virtual int64_t GetFirstTs()=0;
	virtual uint32_t GetUid()=0;
	virtual void NotifyPathUsable(uint8_t pid)=0;
	virtual Ptr<PathSender>GetPathInfo(uint8_t)=0;
	virtual void PacketSchedule(uint32_t,uint8_t)=0;
	virtual void OnNetworkChanged(uint8_t pid,
								  uint32_t bitrate_bps,
								  uint8_t fraction_loss,
								  int64_t rtt_ms)=0;
	virtual void SendVideo(uint8_t payload_type,int ftype,void *data,uint32_t len)=0;
	virtual void SetSchedule(Schedule*)=0;
	virtual void SetRateControl(RateControl *)=0;
    virtual void RegisterPacketSource(VideoSource *v)=0;
    virtual void StopCallback()=0;
};
class SessionInterface{
public:
	virtual ~SessionInterface(){}
	virtual void SendVideo(uint8_t payload_type,int ftype,void *data,uint32_t len)=0;
	virtual bool Fd2Addr(su_socket,su_addr *)=0;
	virtual void PathStateForward(int type,int value)=0;
};
#define MAX_SPLIT_NUMBER	1024
uint16_t FrameSplit(uint16_t splits[], size_t size);
}
#endif /* SIM_TRANSPORT_MPCOMMON_H_ */
