#include "t_trace.h"
#include "ns3/simulator.h"
#include <unistd.h>
#include <memory.h>
namespace ns3{
TTrace::~TTrace(){
	Close();
}
void TTrace::OpenTraceRateFile(std::string n){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ n+"_t_rate.txt";
	m_rate.open(path.c_str(), std::fstream::out);
}
void TTrace::CloseRateFile(){
	if(m_rate.is_open()){
		m_rate.close();
	}
}
void TTrace::OpenTracePendingLenFile(std::string n){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ n+"_t_pend.txt";
	m_pending_len.open(path.c_str(), std::fstream::out);
}
void TTrace::ClosePengingLen(){
	if(m_pending_len.is_open()){
		m_pending_len.close();
	}
}
void TTrace::RecvPendingLen(uint32_t len){
	char line [256];
	memset(line,0,256);
	float now=Simulator::Now().GetSeconds();
	sprintf(line, "%f %16d",now,len);
	if(m_pending_len.is_open()){
		m_pending_len<<line<<std::endl;
	}
}
void TTrace::RecvRate(uint32_t bps){
	char line [256];
	memset(line,0,256);
	float now=Simulator::Now().GetSeconds();
	sprintf(line, "%f %16d",now,bps);
	if(m_rate.is_open()){
		m_rate<<line<<std::endl;
	}
}
void TTrace::Close(){
	CloseRateFile();
	ClosePengingLen();
}
}




