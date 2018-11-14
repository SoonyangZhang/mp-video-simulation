#include "ns3/core-module.h"
#include "ns3/mp-video-module.h"
#include "ns3/webrtc-ns3-module.h"
#include "ns3/log.h"
#include <string>
//#include "ns3/t_pacer_sender.h"
using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("Test_Pace");
uint64_t startTime=0;
uint64_t stopTime=20;
int main(){
    rtc::LogMessage::SetLogToStderr(true);
    LogComponentEnable("Test_Pace",LOG_LEVEL_ALL);
    LogComponentEnable("TPace",LOG_LEVEL_ALL);
    SetClockForWebrtc();//that's a must
    TPace pace;
    RateStatistics rateobserver;
    pace.RegisterObserver(&rateobserver);
    TGenerator *generator=new TGenerator(1500000,25);
    generator->RegisterTarget(&pace);
    std::string name=std::string("pace_test");
    TTrace trace;
    trace.OpenTracePendingLenFile(name);
    trace.OpenTraceRateFile(name);
    rateobserver.SetTraceRate(MakeCallback(&TTrace::RecvRate,&trace));
    pace.SetPendingLenTrace(MakeCallback(&TTrace::RecvPendingLen,&trace));
    generator->Start();
    Simulator::Stop (Seconds(stopTime + 10.0));
    Simulator::Run ();
    Simulator::Destroy();
    generator->Stop();
    delete generator;
}
