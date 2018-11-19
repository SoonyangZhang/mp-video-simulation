#ifndef MULTIPATH_FAKEVIDEOGENERATOR_H_
#define MULTIPATH_FAKEVIDEOGENERATOR_H_
#include "aggregaterate.h"
#include "roundrobinschedule.h"
#include "scaleschedule.h"
#include "wrrschedule.h"
#include "balancecostschedule.h"
#include "edcldschedule.h"
#include "sflschedule.h"
#include "waterfillingschedule.h"
#include "mpcommon.h"
#include "videosource.h"
#include "ns3/event-id.h"
#include "ns3/callback.h"
namespace ns3{
//rate_  in bps;
class FakeVideoGenerator :public RateChangeCallback
,public VideoSource{
public:
	FakeVideoGenerator(uint32_t minbitrate,uint32_t fs);
	~FakeVideoGenerator(){}
	void ChangeRate(uint32_t bitrate) override;
	void RegisterSender(SenderInterface *s);
	void SendFrame();
	void Generate();
	void Start() override;
	void Stop() override;
	typedef Callback<void,uint32_t> RateCallback;
	void SetRateTraceCallback(RateCallback cb){
		m_rate_cb_=cb;
	}
private:
	uint32_t min_bitrate_;
	uint32_t fs_;
	uint32_t rate_;
	uint32_t first_ts_;
	uint32_t last_send_ts_;
	uint32_t delta_;
	uint32_t frame_id_;
	bool running_;
	SenderInterface *sender_;
	AggregateRate ratecontrol_;
	//RoundRobinSchedule schedule_;
    //ScaleSchedule schedule_{CostType::c_intant};
    //WrrSchedule schedule_{CostType::c_intant};
    //BalanceCostSchedule schedule_{CostType::c_intant};
    //EDCLDSchedule schedule_{0.2};
    //SFLSchedule schedule_;
    WaterFillingSchedule schedule_;
	EventId m_timer;
	RateCallback m_rate_cb_;
};
}



#endif /* MULTIPATH_FAKEVIDEOGENERATOR_H_ */
