#include "fakevideoconsumer.h"
#include <string>
#include <ns3/simulator.h>
#include "ns3/log.h"
#include <stdio.h>
#include <string>
#include <unistd.h>
namespace ns3{
NS_LOG_COMPONENT_DEFINE ("FakeVideoConsumer");
FakeVideoConsumer::FakeVideoConsumer(){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
    std::string tracepath = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/" + "mpvideo"+"_recv.txt";
	m_traceRecvFile.open(tracepath.c_str(), std::fstream::out);
    last_=0;
}
void FakeVideoConsumer::ForwardUp(uint32_t fid,uint8_t*data,
			uint32_t len,uint32_t recv,uint32_t total){
    uint32_t now=Simulator::Now().GetMilliSeconds();
    if(last_==0){
        last_=now;
    }
    uint32_t delta=now-last_;
    last_=now;
    float time=Simulator::Now().GetSeconds();
	NS_LOG_INFO("recv video "<<std::to_string(fid)<< " total "<<std::to_string(total)
                <<" recv "<<std::to_string(recv));
	char line [255];
	memset(line,0,255);
	sprintf (line, "%16f %16d %16d %16d %16d",
			time,delta,fid,recv,total);
	m_traceRecvFile<<line<<std::endl;
}
}




