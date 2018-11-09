#include <unistd.h>
#include <memory.h>
#include "mptrace.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
namespace ns3{
NS_LOG_COMPONENT_DEFINE ("Mptrace");
Mptrace::~Mptrace(){
	Close();
}
void Mptrace::OpenTraceFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string gappath = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"_gap.txt";
	m_frameGap.open(gappath.c_str(), std::fstream::out);
}
void Mptrace::CloseTraceFile(){
	Close();
}
void Mptrace::FrameRecvGap(uint32_t fid,uint32_t duration){
	char line [255];
	memset(line,0,255);
	sprintf (line, "%16d %16d",
			fid,duration);
	m_frameGap<<line<<std::endl;
}
void Mptrace::Close(){
	if(m_frameGap.is_open()){
		m_frameGap.close();
	}
}
}


