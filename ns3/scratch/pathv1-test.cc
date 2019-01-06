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
NS_LOG_COMPONENT_DEFINE ("Webrtc-Tcp");
const uint32_t TOPO_DEFAULT_BW     = 4000000;    // in bps: 1Mbps
const uint32_t TOPO_DEFAULT_PDELAY =      100;    // in ms:   50ms
const uint32_t TOPO_DEFAULT_QDELAY =     200;    // in ms:  300ms
const uint32_t DEFAULT_PACKET_SIZE = 1000;
const static uint32_t rateArray[]=
{
2000000,
1500000,
1000000,
 500000,
1000000,
1500000,
};
class ChangeBw
{
public:
	ChangeBw(Ptr<NetDevice> netdevice)
	{
	m_total=sizeof(rateArray)/sizeof(rateArray[0]);
	m_netdevice=netdevice;
	}
	//ChangeBw(){}
	~ChangeBw(){}
	void Start()
	{
		Time next=Seconds(m_gap);
		m_timer=Simulator::Schedule(next,&ChangeBw::ChangeRate,this);		
	}
	void ChangeRate()
	{
		if(m_timer.IsExpired())
		{
		NS_LOG_INFO(Simulator::Now().GetSeconds()<<" "<<rateArray[m_index]/1000);
		//Config::Set ("/ChannelList/0/$ns3::PointToPointChannel/DataRate",DataRateValue (rateArray[m_index]));
		PointToPointNetDevice *device=static_cast<PointToPointNetDevice*>(PeekPointer(m_netdevice));
		device->SetDataRate(DataRate(rateArray[m_index]));
		m_index=(m_index+1)%m_total;
		Time next=Seconds(m_gap);
		m_timer=Simulator::Schedule(next,&ChangeBw::ChangeRate,this);
		}

	}
private:
	uint32_t m_index{1};
	uint32_t m_gap{20};
	uint32_t m_total{6};
	Ptr<NetDevice>m_netdevice;
	EventId m_timer;
};
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
static double simDuration=300;
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
    LogComponentEnable("PathSenderV1",LOG_LEVEL_ALL);
    LogComponentEnable("PathReceiverV1",LOG_LEVEL_ALL);
    LogComponentEnable("MpSenderV1",LOG_LEVEL_ALL);
	uint64_t linkBw   = TOPO_DEFAULT_BW;
    uint32_t msDelay  = TOPO_DEFAULT_PDELAY;
    uint32_t msQDelay = TOPO_DEFAULT_QDELAY;
    NodeContainer nodes = BuildExampleTopo (linkBw, msDelay, msQDelay);
    uint32_t min_bps=500000;
    uint32_t max_bps=2000000;
    Ptr<PathSenderV1> spath1_1=CreateObject<PathSenderV1>(min_bps,max_bps);
	Ptr<PathReceiverV1> rpath1_1=CreateObject<PathReceiverV1>();
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
    spath1_1->set_path_id(1);
    rpath1_1->set_path_id(1);
    rpath1_1->RegisterPathSender(spath1_1);//get first ts;

    std::string schedule_name=std::string("sfl-");
    //std::shared_ptr<ScheduleV1> schedule1(new EDCLDScheduleV1(0.8));
    //std::shared_ptr<ScheduleV1> schedule1(new IdealSchedule());
    //std::shared_ptr<ScheduleV1> schedule1(new BCScheduleV1());
    std::shared_ptr<ScheduleV1> schedule1(new SFLScheduleV1());
    std::string log_name_1_1=schedule_name+std::string("sender-1-1");
    TraceSenderV1 trace1;
    trace1.OpenTraceRateFile(log_name_1_1);
    trace1.OpenTraceLossFile(log_name_1_1);
    trace1.OpenTraceSumRateFile(log_name_1_1);
    trace1.OpenTraceTimeOffsetFile(log_name_1_1);
    spath1_1->SetRateTraceFunc(MakeCallback(&TraceSenderV1::OnRate,&trace1));
    spath1_1->SetLossTraceFunc(MakeCallback(&TraceSenderV1::OnLoss,&trace1));
    spath1_1->SetTimeOffsetTraceFunc(MakeCallback(&TraceSenderV1::OnTimeOffset,&trace1));  
    

//add second path

	linkBw   = 2000000;
    msDelay  = 50;
    msQDelay = 200;
    NodeContainer nodes1 = BuildExampleTopo (linkBw, msDelay, msQDelay);
    min_bps=500000;
    max_bps=2000000;
    Ptr<PathSenderV1> spath1_2=CreateObject<PathSenderV1>(min_bps,max_bps);
	Ptr<PathReceiverV1> rpath1_2=CreateObject<PathReceiverV1>();
    nodes1.Get(0)->AddApplication (spath1_2);
    nodes1.Get(1)->AddApplication (rpath1_2);
    spath1_2->Bind(client_port);
    rpath1_2->Bind(servPort);
    spath1_2->SetStartTime (Seconds (appStart));
    spath1_2->SetStopTime (Seconds (appStop));
    rpath1_2->SetStartTime (Seconds (appStart));
    rpath1_2->SetStopTime (Seconds (appStop));
    remote=rpath1_2->GetLocalAddress();
    spath1_2->ConfigurePeer(remote.GetIpv4(),remote.GetPort());
    spath1_2->set_path_id(2);
    rpath1_2->set_path_id(2);
    rpath1_2->RegisterPathSender(spath1_2);//get first ts;
    

    
    std::string log_name_1_2=schedule_name+std::string("sender-1-2");
    TraceSenderV1 trace12;
    trace12.OpenTraceRateFile(log_name_1_2);
    trace12.OpenTraceLossFile(log_name_1_2);
    trace12.OpenTraceSumRateFile(log_name_1_2);
    trace12.OpenTraceTimeOffsetFile(log_name_1_2);
    spath1_2->SetRateTraceFunc(MakeCallback(&TraceSenderV1::OnRate,&trace12));
    spath1_2->SetLossTraceFunc(MakeCallback(&TraceSenderV1::OnLoss,&trace12));
    spath1_2->SetTimeOffsetTraceFunc(MakeCallback(&TraceSenderV1::OnTimeOffset,&trace12));
    
    MpSenderV1 mpsender1;
    IdealEncoder encoder1(500000,500000,2000000);
    mpsender1.set_encoder(&encoder1);
    mpsender1.set_schedule(schedule1.get());
    mpsender1.RegisterSender(spath1_1);
    
    mpsender1.RegisterSender(spath1_2);
    
    mpsender1.SetSumRateTraceFun(MakeCallback(&TraceSenderV1::OnSumRate,&trace1));
    MpReceiverV1 mpreceiver1;
    mpreceiver1.RegisteReceiver(rpath1_1);
    
    mpreceiver1.RegisteReceiver(rpath1_2);

    TraceReceiverV1 recv_trace1;
    recv_trace1.OpenTraceRecvFrameFile(log_name_1_2);
    mpreceiver1.SetFrameDelayTraceFunc(MakeCallback(&TraceReceiverV1::OnRecvFrame,&recv_trace1));
 


//second sender   
    Ptr<PathSenderV1> spath2_1=CreateObject<PathSenderV1>(min_bps,max_bps);
	Ptr<PathReceiverV1> rpath2_1=CreateObject<PathReceiverV1>();
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
    spath2_1->set_path_id(1);
    rpath2_1->set_path_id(1);
    rpath2_1->RegisterPathSender(spath2_1);
    schedule_name=std::string("wrr-");
    std::string log_name_2_1=schedule_name+std::string("sender-2-1");
    TraceSenderV1 trace2;
    trace2.OpenTraceRateFile(log_name_2_1);
    trace2.OpenTraceLossFile(log_name_2_1);
    trace2.OpenTraceSumRateFile(log_name_2_1);
    trace2.OpenTraceTimeOffsetFile(log_name_2_1);
    spath2_1->SetRateTraceFunc(MakeCallback(&TraceSenderV1::OnRate,&trace2));
    spath2_1->SetLossTraceFunc(MakeCallback(&TraceSenderV1::OnLoss,&trace2));
    spath2_1->SetTimeOffsetTraceFunc(MakeCallback(&TraceSenderV1::OnTimeOffset,&trace2));
    MpSenderV1 mpsender2;
    IdealEncoder encoder2(500000,500000,2000000);
    mpsender2.set_encoder(&encoder2);
    IdealSchedule schedule2;
    mpsender2.set_schedule(&schedule2);
    mpsender2.RegisterSender(spath2_1);
    mpsender2.SetSumRateTraceFun(MakeCallback(&TraceSenderV1::OnSumRate,&trace2));
    MpReceiverV1 mpreceiver2;
    mpreceiver2.RegisteReceiver(rpath2_1);
    TraceReceiverV1 recv_trace2;
    recv_trace2.OpenTraceRecvFrameFile(log_name_2_1);
    mpreceiver2.SetFrameDelayTraceFunc(MakeCallback(&TraceReceiverV1::OnRecvFrame,&recv_trace2));


//third sender
    Ptr<PathSenderV1> spath3_1=CreateObject<PathSenderV1>(min_bps,max_bps);
	Ptr<PathReceiverV1> rpath3_1=CreateObject<PathReceiverV1>();
    nodes.Get(0)->AddApplication (spath3_1);
    nodes.Get(1)->AddApplication (rpath3_1);
    spath3_1->Bind(client_port+2);
    rpath3_1->Bind(servPort+2);
    spath3_1->SetStartTime (Seconds (appStart+80));
    spath3_1->SetStopTime (Seconds (appStop));
    rpath3_1->SetStartTime (Seconds (appStart+80));
    rpath3_1->SetStopTime (Seconds (appStop));
    remote=rpath3_1->GetLocalAddress();
    spath3_1->ConfigurePeer(remote.GetIpv4(),remote.GetPort());
    spath3_1->set_path_id(1);
    rpath3_1->set_path_id(1);
    rpath3_1->RegisterPathSender(spath3_1);
    schedule_name=std::string("wrr-");
    std::string log_name_3_1=schedule_name+std::string("sender-3-1");
    TraceSenderV1 trace3;
    trace3.OpenTraceRateFile(log_name_3_1);
    trace3.OpenTraceLossFile(log_name_3_1);
    trace3.OpenTraceSumRateFile(log_name_3_1);
    trace3.OpenTraceTimeOffsetFile(log_name_3_1);
    spath3_1->SetRateTraceFunc(MakeCallback(&TraceSenderV1::OnRate,&trace3));
    spath3_1->SetLossTraceFunc(MakeCallback(&TraceSenderV1::OnLoss,&trace3));
    spath3_1->SetTimeOffsetTraceFunc(MakeCallback(&TraceSenderV1::OnTimeOffset,&trace3));
    MpSenderV1 mpsender3;
    IdealEncoder encoder3(500000,500000,2000000);
    mpsender3.set_encoder(&encoder3);
    IdealSchedule schedule3;
    mpsender3.set_schedule(&schedule3);
    mpsender3.RegisterSender(spath3_1);
    mpsender3.SetSumRateTraceFun(MakeCallback(&TraceSenderV1::OnSumRate,&trace3));
    MpReceiverV1 mpreceiver3;
    mpreceiver3.RegisteReceiver(rpath3_1);
    TraceReceiverV1 recv_trace3;
    recv_trace3.OpenTraceRecvFrameFile(log_name_3_1);
    mpreceiver3.SetFrameDelayTraceFunc(MakeCallback(&TraceReceiverV1::OnRecvFrame,&recv_trace3));

    
//    InstallTcp(nodes.Get(0),nodes.Get(1),2222,40,150);
    Simulator::Stop (Seconds(simDuration));
    Simulator::Run ();
    Simulator::Destroy();	
	return 0;
}



