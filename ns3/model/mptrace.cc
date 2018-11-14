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
	char line [256];
	memset(line,0,256);
	sprintf (line, "%16d %16d",
			fid,duration);
	m_frameGap<<line<<std::endl;
}
void Mptrace::Close(){
	if(m_frameGap.is_open()){
		m_frameGap.close();
	}
}
TraceDelayInfo::~TraceDelayInfo(){
	Close();
}
void TraceDelayInfo::OpenTracePendingDelayFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string pendingpath = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"_pend.txt";
	if(!m_pending.is_open()){
		m_pending.open(pendingpath.c_str(), std::fstream::out);
	}
}
void TraceDelayInfo::OpenTraceOwdFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string owdpath = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"_owd.txt";
	if(!m_owd.is_open()){
		m_owd.open(owdpath.c_str(), std::fstream::out);
	}
}
void TraceDelayInfo::OpenTraceRateFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string owdpath = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"_rate.txt";
	if(!m_rate.is_open()){
		m_rate.open(owdpath.c_str(), std::fstream::out);
	}
}
void TraceDelayInfo::ClosePendingDelayFile(){
	if(m_pending.is_open()){
		m_pending.close();
	}
}
void TraceDelayInfo::CloseOwdFile(){
	if(m_owd.is_open()){
		m_owd.close();
	}
}
void TraceDelayInfo::CloseRateFile(){
	if(m_rate.is_open()){
		m_rate.close();
	}
}
void TraceDelayInfo::RecvOwd(uint32_t packet_id,uint32_t owd){
	char line [256];
	memset(line,0,256);
	float now=Simulator::Now().GetSeconds();
	sprintf(line, "%f %16d %16d",now,packet_id,
			owd);
	if(m_owd.is_open()){
		m_owd<<line<<std::endl;
	}
}
void TraceDelayInfo::RecvPendDelay(uint32_t packet_id,uint32_t sent_ts){
	char line [256];
	memset(line,0,256);
	float now=Simulator::Now().GetSeconds();
	sprintf(line, "%f %16d %16d",now,packet_id,
			sent_ts);
	if(m_pending.is_open()){
		m_pending<<line<<std::endl;
	}
}
void TraceDelayInfo::RecvRate(uint32_t bps){
	char line [256];
	memset(line,0,256);
	float now=Simulator::Now().GetSeconds();
	float rate=(float)(bps)/1000;
	sprintf(line, "%f %16f",now,rate);
	if(m_rate.is_open()){
		m_rate<<line<<std::endl;
	}
}
void TraceDelayInfo::Close(){
	ClosePendingDelayFile();
	CloseOwdFile();
	CloseRateFile();
}
}


