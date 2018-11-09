#ifndef MULTIPATH_FAKEVIDEOCONSUMER_H_
#define MULTIPATH_FAKEVIDEOCONSUMER_H_
#include "mpcommon.h"
#include <iostream>
#include <fstream>
namespace ns3{
class FakeVideoConsumer:public NetworkDataConsumer{
public:
    FakeVideoConsumer();
	void ForwardUp(uint32_t fid,uint8_t*data,
			uint32_t len,uint32_t recv,uint32_t total) override;
private:
    std::fstream m_traceRecvFile;
    uint32_t last_;
};
}




#endif /* MULTIPATH_FAKEVIDEOCONSUMER_H_ */
