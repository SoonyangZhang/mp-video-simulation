#ifndef NS3_MPVIDEO_FUN_TEST_RATESTATISTICS_H_
#define NS3_MPVIDEO_FUN_TEST_RATESTATISTICS_H_
#include "t_interface.h"
#include "ns3/callback.h"
namespace ns3{
class RateStatistics:public PacketSentObserver{
public:
	RateStatistics(){}
	~RateStatistics(){}
	void OnSent(uint32_t len) override;
	typedef Callback<void,uint32_t> RateCallback;
	void SetTraceRate(RateCallback cb){m_rate=cb;};
private:
	void Reset();
	uint32_t begin_{0};
	RateCallback m_rate;
	uint32_t len_{0};
};
}
#endif /* NS3_MPVIDEO_FUN_TEST_RATESTATISTICS_H_ */
