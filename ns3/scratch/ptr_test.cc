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
NS_LOG_COMPONENT_DEFINE ("ptr-test");
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
#define MIN_SEND_BITRATE (20 * 8 * 1000)
#include "rtc_base/logging.h"

int main(int argc, char *argv[]){
    rtc::LogMessage::SetLogToStderr(true);
    LogComponentEnable("ptr-test",LOG_LEVEL_ALL);
    //LogComponentEnable("MultipathSender",LOG_LEVEL_ALL);
    //LogComponentEnable("PathSender",LOG_LEVEL_ALL);
    //LogComponentEnable("PathReceiver",LOG_LEVEL_ALL);
    //LogComponentEnable("FakeVideoGenerator",LOG_LEVEL_ALL);
    //LogComponentEnable("ScaleSchedule",LOG_LEVEL_ALL);
    //LogComponentEnable("WrrSchedule",LOG_LEVEL_ALL);
    //LogComponentEnable("AggregateRate",LOG_LEVEL_ALL);
    //LogComponentEnable("WaterFillingSchedule",LOG_LEVEL_ALL);
   // LogComponentEnable("SFLSchedule",LOG_LEVEL_ALL);
    //LogComponentEnable("EDCLDSchedule",LOG_LEVEL_ALL);
    //LogComponentEnable("FakeVideoGenerator",LOG_LEVEL_ALL);
    std::string scheduleType;//=std::string("wrr");
    std::string test_case=std::string("10");
    std::string mode=std::string("normal");//std::string("ownpace");//std::string("oracle");
    CommandLine cmd;
    cmd.AddValue ("st", "schedule type", scheduleType);
    cmd.AddValue ("cs", "test_case", test_case);
    cmd.AddValue ("md", "mode", mode);
    cmd.Parse (argc, argv);
    NS_LOG_INFO("case "<<test_case);
    SetClockForWebrtc();//that's a must
	Ptr<PathSender> spath1=CreateObject<PathSender>();
	Ptr<PathReceiver> rpath1=CreateObject<PathReceiver>();
	MultipathSender sender(11);
    std::string schedule_prefix=scheduleType;
    std::string gapname=schedule_prefix+std::string("_")+test_case;
    TraceReceive trace;
    trace.OpenTraceGapFile(gapname);
    trace.OpenTraceRecvBufFile(gapname);
    trace.OpenTraceFrameInfoFile(gapname);
    MultipathReceiver receiver(12);
    receiver.SetGapCallback(MakeCallback(&TraceReceive::RecvGap,&trace));
    receiver.SetRecvBufLenTrace(MakeCallback(&TraceReceive::RecvBufLen,&trace));
    receiver.SetFrameInfoTrace(MakeCallback(&TraceReceive::RecvFrameInfo,&trace));
    NodeContainer nodes;
    NodeContainer nodes2;
    uint64_t bw1=0;
    uint64_t bw2=0;
    if(test_case==std::string("1")){
        bw1=2000000;
        bw2=1000000;
        nodes=BuildExampleTopo(bw1,100,200);
        nodes2=BuildExampleTopo(bw2,150,200);    
    }else if(test_case==std::string("2")){
    	bw1=2000000;
    	bw2=1000000;
    	nodes=BuildExampleTopo(bw1,100,200);
    	nodes2=BuildExampleTopo(bw2,100,200);
    }else if(test_case==std::string("3")){
    	bw1=3000000;
    	bw2=1000000;
    	nodes=BuildExampleTopo(bw1,150,200);
    	nodes2=BuildExampleTopo(bw2,100,200);
    }else if(test_case==std::string("4")){
    	bw1=2000000;
    	bw2=1000000;
    	nodes=BuildExampleTopo(bw1,50,200);
    	nodes2=BuildExampleTopo(bw2,100,200);
    }else if(test_case==std::string("5")){
    	bw1=1000000;
    	bw2=1000000;
    	nodes=BuildExampleTopo(bw1,50,200);
    	nodes2=BuildExampleTopo(bw2,100,200);
    }else if(test_case==std::string("6")){
    	bw1=3000000;
    	bw2=3000000;
    	nodes=BuildExampleTopo(bw1,50,200);
    	nodes2=BuildExampleTopo(bw2,100,200);
    }else if(test_case==std::string("7")){
    	bw1=2000000;
    	bw2=3000000;
    	nodes=BuildExampleTopo(bw1,30,200);
    	nodes2=BuildExampleTopo(bw2,50,200);
    }else if(test_case==std::string("8")){
    	bw1=4000000;
    	bw2=2000000;
    	nodes=BuildExampleTopo(bw1,30,200);
    	nodes2=BuildExampleTopo(bw2,100,200);
    }else if(test_case==std::string("9")){
    	bw1=2000000;
    	bw2=2000000;
    	nodes=BuildExampleTopo(bw1,30,200);
    	nodes2=BuildExampleTopo(bw2,100,200);
    }else if(test_case==std::string("10")){
    	bw1=3000000;
    	bw2=2000000;
    	nodes=BuildExampleTopo(bw1,100,300);
    	nodes2=BuildExampleTopo(bw2,100,200);
    }
    
    //NodeContainer nodes=BuildExampleTopo(2000000,100,200); //oracle 1
    //NodeContainer nodes=BuildExampleTopo(2000000,100,200);// oracle 2
    //NodeContainer nodes=BuildExampleTopo(3000000,150,200);// oracle 3
    //NodeContainer nodes=BuildExampleTopo(2000000,50,200);// oracle 4
    //NodeContainer nodes=BuildExampleTopo(1000000,50,200);// oracle 5
    //NodeContainer nodes=BuildExampleTopo(3000000,50,200);// oracle 6
    //NodeContainer nodes=BuildExampleTopo(2000000,30,200);// oracle 7
    //NodeContainer nodes=BuildExampleTopo(4000000,30,200);// oracle 8
    //NodeContainer nodes=BuildExampleTopo(2000000,30,200);// oracle 9
    nodes.Get(0)->AddApplication (spath1);
    nodes.Get(1)->AddApplication (rpath1);
    spath1->Bind(1234);
    rpath1->Bind(4321);
    spath1->SetMaxBw(bw1);

	Ptr<PathSender> spath2=CreateObject<PathSender>();
	Ptr<PathReceiver> rpath2=CreateObject<PathReceiver>();
    nodes2.Get(0)->AddApplication (spath2);
    nodes2.Get(1)->AddApplication (rpath2);
    spath2->Bind(1234);
    rpath2->Bind(4321);
    spath2->SetMaxBw(bw2);
    if(mode==std::string("oracle")){
    spath1->SetOracleRate(bw1*4/5);//80% bw utility
    rpath1->SetOracleMode(); 

    spath2->SetOracleRate(bw2*4/5);//80% bw utility
    rpath2->SetOracleMode();   
    }else if(mode==std::string("ownpace")){
    spath1->SetOracleWithOwnPace(bw1*4/5);//80% bw utility
    rpath1->SetOracleMode(); 

    spath2->SetOracleWithOwnPace(bw2*4/5);//80% bw utility
    rpath2->SetOracleMode();    
    }else{
    
    }


    //NodeContainer nodes2=BuildExampleTopo(1000000,150,200); // 1
    //NodeContainer nodes2=BuildExampleTopo(1000000,100,200); //2 
    //NodeContainer nodes2=BuildExampleTopo(1000000,100,200); // 3
    //NodeContainer nodes2=BuildExampleTopo(2000000,100,200);// 4
    //NodeContainer nodes2=BuildExampleTopo(1000000,100,200);// 5
    //NodeContainer nodes2=BuildExampleTopo(3000000,100,200); //6
    //NodeContainer nodes2=BuildExampleTopo(3000000,50,200);//7
    //NodeContainer nodes2=BuildExampleTopo(2000000,100,200);//8
    //NodeContainer nodes2=BuildExampleTopo(2000000,100,200);//9
    //int record_id=1;

    //std::string delay_name=schedule_prefix+std::string("_")+test_case+std::string("_")+std::to_string(record_id);
    //record_id++;
    TraceDelayInfo trace_d_p1;
    //trace_d_p1.OpenTracePendingDelayFile(delay_name);
    //trace_d_p1.OpenTraceOwdFile(delay_name);

    //spath1->SetPendingDelayTrace(MakeCallback(&TraceDelayInfo::RecvPendDelay,&trace_d_p1));
    //rpath1->SetPacketDelayTrace(MakeCallback(&TraceDelayInfo::RecvOwd,&trace_d_p1));
    rpath1->SetSourceEnd(spath1);
    //delay_name=schedule_prefix+std::string("_")+test_case+std::string("_")+std::to_string(record_id);
    //record_id++;
    TraceDelayInfo trace_d_p2;
    //trace_d_p2.OpenTracePendingDelayFile(delay_name);
    //trace_d_p2.OpenTraceOwdFile(delay_name);

    //spath2->SetPendingDelayTrace(MakeCallback(&TraceDelayInfo::RecvPendDelay,&trace_d_p2));
   // rpath2->SetPacketDelayTrace(MakeCallback(&TraceDelayInfo::RecvOwd,&trace_d_p2));
   // rpath2->SetSourceEnd(spath2);

    sender.RegisterPath(spath1);
    receiver.RegisterPath(rpath1);

    sender.RegisterPath(spath2);
    receiver.RegisterPath(rpath2);

    std::string ratename=schedule_prefix+std::string("_")+test_case;
    FakeVideoGenerator source(CodeCType::ideal,MIN_SEND_BITRATE,30);
    source.ConfigureSchedule(schedule_prefix);
    source.RegisterSender(&sender);
    
    trace_d_p1.OpenTraceRateFile(ratename);
    source.SetRateTraceCallback(MakeCallback(&TraceDelayInfo::RecvRate,&trace_d_p1));

    FakeVideoConsumer sink;
    receiver.RegisterDataSink(&sink);
    std::string sinkname=ratename;
    sink.SetTraceName(sinkname);
    std::vector<InetSocketAddress> addr;
    addr.push_back(spath1->GetLocalAddress());
    addr.push_back(rpath1->GetLocalAddress());

    addr.push_back(spath2->GetLocalAddress());
    addr.push_back(rpath2->GetLocalAddress());
    spath1->SetStartTime (Seconds (startTime));
    spath1->SetStopTime (Seconds (stopTime));
    rpath1->SetStartTime (Seconds (startTime));
    rpath1->SetStopTime (Seconds (stopTime));

    spath2->SetStartTime (Seconds (startTime));
    spath2->SetStopTime (Seconds (stopTime));
    rpath2->SetStartTime (Seconds (startTime));
    rpath2->SetStopTime (Seconds (stopTime));
    sender.Connect(addr);
    Simulator::Stop (Seconds(stopTime + 10.0));
    Simulator::Run ();
    Simulator::Destroy();
}
