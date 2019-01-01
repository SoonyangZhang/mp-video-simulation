#ifndef NS3_MPVIDEO_NS_QUIC_TIME_H_
#define NS3_MPVIDEO_NS_QUIC_TIME_H_
#include "net/third_party/quic/core/quic_time.h"
namespace ns3{
class Ns3QuicTime{
public:
static quic::QuicTime Now();
};
}




#endif /* NS3_MPVIDEO_NS_QUIC_TIME_H_ */
