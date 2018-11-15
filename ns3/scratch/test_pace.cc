#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include <iostream>
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "rtc_base/timeutils.h"
#include <unistd.h>
#include "ns3/mp-video-module.h"
#include "ns3/webrtc-ns3-module.h"
#include <string>
using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("Test_Pace");
const uint32_t DEFAULT_PACKET_SIZE = 1000;
static int ip=1;
static NodeContainer BuildExampleTopo (uint64_t bps,
                                       uint32_t msDelay,
                                       uint32_t msQdelay)
{
    NodeContainer nodes;
    nodes.Create (2);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue  (DataRate (bps)));
    pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (msDelay)));
    auto bufSize = std::max<uint32_t> (DEFAULT_PACKET_SIZE, bps * msQdelay / 8000);
    pointToPoint.SetQueue ("ns3::DropTailQueue",
                           "Mode", StringValue ("QUEUE_MODE_BYTES"),
                           "MaxBytes", UintegerValue (bufSize));
    NetDeviceContainer devices = pointToPoint.Install (nodes);

    InternetStackHelper stack;
    stack.Install (nodes);
    Ipv4AddressHelper address;
    std::string nodeip="10.1."+std::to_string(ip)+".0";
    ip++;
    address.SetBase (nodeip.c_str(), "255.255.255.0");
    address.Assign (devices);

    // Uncomment to capture simulated traffic
    // pointToPoint.EnablePcapAll ("rmcat-example");

    // disable tc for now, some bug in ns3 causes extra delay
    TrafficControlHelper tch;
    tch.Uninstall (devices);

    return nodes;
}
uint64_t startTime=0;
uint64_t stopTime=200;
int main(){
    rtc::LogMessage::SetLogToStderr(true);
    LogComponentEnable("Test_Pace",LOG_LEVEL_ALL);
    LogComponentEnable("TSendAgent",LOG_LEVEL_ALL);
    LogComponentEnable("TRecvAgent",LOG_LEVEL_ALL);
    SetClockForWebrtc();//that's a must
    Ptr<TSendAgent> sender=CreateObject<TSendAgent>();
    Ptr<TRecvAgent> receiver=CreateObject<TRecvAgent>();
    
    RateStatistics rateobserver;
    sender->RegisterObserver(&rateobserver);
    TGenerator *generator=new TGenerator(1500000,25);
    generator->RegisterTarget(GetPointer(sender));
    std::string name=std::string("pace_test");
    TTrace trace;
    trace.OpenTracePendingLenFile(name);
    trace.OpenTraceOwdFile(name);
    trace.OpenTraceRtsFile(name);
    rateobserver.SetTraceRate(MakeCallback(&TTrace::RecvRate,&trace));
    sender->SetOwdTrace(MakeCallback(&TTrace::RecvOwd,&trace));
    receiver->SetTraceRecvTs(MakeCallback(&TTrace::RecvRts,&trace));
    NodeContainer nodes=BuildExampleTopo(2000000,100,200);
    nodes.Get(0)->AddApplication (sender);
    nodes.Get(1)->AddApplication (receiver);

    sender->Bind(1234);
    receiver->Bind(4321);

    sender->SetDestination(receiver->GetLocalAddress());

    sender->SetStartTime (Seconds (startTime));
    sender->SetStopTime (Seconds (stopTime));
    receiver->SetStartTime (Seconds (startTime));
    receiver->SetStopTime (Seconds (stopTime));
    generator->Start();
    Simulator::Stop (Seconds(stopTime + 10.0));
    Simulator::Run ();
    Simulator::Destroy();
    generator->Stop();
    delete generator;
}
