#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/bulk-send-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/mp-video-module.h"
#include "ns3/nstime.h"
#include "ns3/mytrace.h"
#include "ns3/simulator.h"
#include <stdarg.h>
#include <string>
#include <memory>
using namespace ns3;
using namespace std;
using namespace zsy;
NS_LOG_COMPONENT_DEFINE ("Mock-Test");
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
/*
	std::string errorModelType = "ns3::RateErrorModel";
  	ObjectFactory factory;
  	factory.SetTypeId (errorModelType);
  	Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
	devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));*/
    return nodes;
}

static void InstallTcp(
                         Ptr<Node> sender,
                         Ptr<Node> receiver,
                         uint16_t port,
                         float startTime,
                         float stopTime
)
{
    // configure TCP source/sender/client
    auto serverAddr = receiver->GetObject<Ipv4> ()->GetAddress (1,0).GetLocal ();
    BulkSendHelper source{"ns3::TcpSocketFactory",
                           InetSocketAddress{serverAddr, port}};
    // Set the amount of data to send in bytes. Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (0));
    source.SetAttribute ("SendSize", UintegerValue (DEFAULT_PACKET_SIZE));

    auto clientApps = source.Install (sender);
    clientApps.Start (Seconds (startTime));
    clientApps.Stop (Seconds (stopTime));

    // configure TCP sink/receiver/server
    PacketSinkHelper sink{"ns3::TcpSocketFactory",
                           InetSocketAddress{Ipv4Address::GetAny (), port}};
    auto serverApps = sink.Install (receiver);
    serverApps.Start (Seconds (startTime));
    serverApps.Stop (Seconds (stopTime));	
	
}
static double simDuration=400;
uint16_t client_port=1234;
uint16_t servPort=4321;
float appStart=0.0;
float appStop=simDuration;

int main(int argc, char *argv[])
{
	Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (0.1));
	Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));

	Config::SetDefault ("ns3::BurstErrorModel::ErrorRate", DoubleValue (0.05));
	Config::SetDefault ("ns3::BurstErrorModel::BurstSize", StringValue ("ns3::UniformRandomVariable[Min=1|Max=3]"));
    LogComponentEnable("MockSender",LOG_LEVEL_ALL);
    LogComponentEnable("MockReceiver",LOG_LEVEL_ALL);
    LogComponentEnable("MpSenderV1",LOG_LEVEL_ALL);

	uint64_t linkBw_1   = 4000000;
    uint32_t msDelay_1  = 100;
    uint32_t msQDelay_1 = 200;

    NodeContainer nodes = BuildExampleTopo (linkBw_1, msDelay_1, msQDelay_1);
    uint32_t min_bps=500000;
    uint32_t max_bps=3000000;
    int cc_counter=1;
    Ptr<MockSender> spath1_1=CreateObject<MockSender>(min_bps,max_bps,cc_counter);
	Ptr<MockReceiver> rpath1_1=CreateObject<MockReceiver>();
    nodes.Get(0)->AddApplication (spath1_1);
    nodes.Get(1)->AddApplication (rpath1_1);
    spath1_1->Bind(client_port);
    rpath1_1->Bind(servPort);
    spath1_1->SetStartTime (Seconds (appStart));
    spath1_1->SetStopTime (Seconds (appStop));
    rpath1_1->SetStartTime (Seconds (appStart));
    rpath1_1->SetStopTime (Seconds (appStop));
    InetSocketAddress remote=rpath1_1->GetLocalAddress();
    spath1_1->ConfigurePeer(remote.GetIpv4(),remote.GetPort());

    std::string log_name_1=std::string("mock-1");
    TraceSenderV1 trace1;
    trace1.OpenTraceRateFile(log_name_1);
    trace1.OpenTraceLossFile(log_name_1);
    spath1_1->SetRateTraceFunc(MakeCallback(&TraceSenderV1::OnRate,&trace1));
    spath1_1->SetLossTraceFunc(MakeCallback(&TraceSenderV1::OnLoss,&trace1));

    cc_counter++;
    Ptr<MockSender> spath2_1=CreateObject<MockSender>(min_bps,max_bps,cc_counter);
	Ptr<MockReceiver> rpath2_1=CreateObject<MockReceiver>();
    nodes.Get(0)->AddApplication (spath2_1);
    nodes.Get(1)->AddApplication (rpath2_1);
    spath2_1->Bind(client_port+1);
    rpath2_1->Bind(servPort+1);
    spath2_1->SetStartTime (Seconds (appStart+40));
    spath2_1->SetStopTime (Seconds (appStop));
    rpath2_1->SetStartTime (Seconds (appStart+40));
    rpath2_1->SetStopTime (Seconds (appStop));
    remote=rpath2_1->GetLocalAddress();
    spath2_1->ConfigurePeer(remote.GetIpv4(),remote.GetPort());

    std::string log_name_2=std::string("mock-2");
    TraceSenderV1 trace2;
    trace2.OpenTraceRateFile(log_name_2);
    trace2.OpenTraceLossFile(log_name_2);
    spath2_1->SetRateTraceFunc(MakeCallback(&TraceSenderV1::OnRate,&trace2));
    spath2_1->SetLossTraceFunc(MakeCallback(&TraceSenderV1::OnLoss,&trace2));


    cc_counter++;
    Ptr<MockSender> spath3_1=CreateObject<MockSender>(min_bps,max_bps,cc_counter);
	Ptr<MockReceiver> rpath3_1=CreateObject<MockReceiver>();
    nodes.Get(0)->AddApplication (spath3_1);
    nodes.Get(1)->AddApplication (rpath3_1);
    spath3_1->Bind(client_port+2);
    rpath3_1->Bind(servPort+2);
    spath3_1->SetStartTime (Seconds (appStart+100));
    spath3_1->SetStopTime (Seconds (appStop));
    rpath3_1->SetStartTime (Seconds (appStart+100));
    rpath3_1->SetStopTime (Seconds (appStop));
    remote=rpath3_1->GetLocalAddress();
    spath3_1->ConfigurePeer(remote.GetIpv4(),remote.GetPort());

    std::string log_name_3=std::string("mock-3");
    TraceSenderV1 trace3;
    trace3.OpenTraceRateFile(log_name_3);
    trace3.OpenTraceLossFile(log_name_3);
    spath3_1->SetRateTraceFunc(MakeCallback(&TraceSenderV1::OnRate,&trace3));
    spath3_1->SetLossTraceFunc(MakeCallback(&TraceSenderV1::OnLoss,&trace3));
    
    //InstallTcp(nodes.Get(0),nodes.Get(1),4444,100,200);
    Simulator::Stop (Seconds(simDuration));
    Simulator::Run ();
    Simulator::Destroy();

}
