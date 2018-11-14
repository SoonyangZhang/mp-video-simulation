#ifndef NS3_MPVIDEO_FUN_TEST_T_GENERATOR_H_
#define NS3_MPVIDEO_FUN_TEST_T_GENERATOR_H_
#include "ns3/simulator.h"
#include "t_interface.h"
#include "ns3/event-id.h"
#include "ns3/callback.h"
namespace ns3{
class TGenerator{
public:
	TGenerator(uint32_t start_bps,uint32_t fs);
	~TGenerator(){}
	void ChangeRate(uint32_t bitrate);
	void RegisterTarget(DataTarget *t);
	void SendFrame();
	void Generate();
	void Start();
	void Stop() ;
	typedef Callback<void,uint32_t> RateCallback;
	void SetRateTraceCallback(RateCallback cb){
		m_rate_cb_=cb;
	}
private:
	uint32_t min_bitrate_;
	uint32_t fs_;
	uint32_t delta_;
	uint32_t rate_;
	int64_t last_send_ts_{0};
	EventId m_timer;
	RateCallback m_rate_cb_;
	DataTarget *t_{NULL};
	bool running_{false};
};
}



#endif /* NS3_MPVIDEO_FUN_TEST_T_GENERATOR_H_ */
