#include "trace_v1.h"
#include <unistd.h>
#include "ns3/simulator.h"
namespace ns3{
TraceSenderV1::~TraceSenderV1(){
	Close();
}
void TraceSenderV1::OpenTraceRateFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"-rate.txt";
	m_rate.open(path.c_str(), std::fstream::out);
}
void TraceSenderV1::OnRate(uint32_t bps){
	if(m_rate.is_open()){
		char line [256];
		memset(line,0,256);
		float now=Simulator::Now().GetSeconds();
		float rate=(float)(bps)/1000;
		sprintf(line, "%f %16f",now,rate);
		m_rate<<line<<std::endl;
	}
}
void TraceSenderV1::OpenTraceLossFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string gappath = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"-loss.txt";
	m_loss.open(gappath.c_str(), std::fstream::out);
}
void TraceSenderV1::OnLoss(uint32_t seq,uint32_t rtt){
	if(m_loss.is_open()){
		char line [256];
		memset(line,0,256);
		float now=Simulator::Now().GetSeconds();
		sprintf(line, "%f %16d %16d",now,rtt,seq);
		m_loss<<line<<std::endl;
	}
}
void TraceSenderV1::OpenTraceSumRateFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"-sum-rate.txt";
	m_sumrate.open(path.c_str(), std::fstream::out);
}
void TraceSenderV1::OnSumRate(uint32_t bps){
	if(m_sumrate.is_open()){
		char line [256];
		memset(line,0,256);
		float now=Simulator::Now().GetSeconds();
		float rate=(float)(bps)/1000;
		sprintf(line, "%f %16f",now,rate);
		m_sumrate<<line<<std::endl;
	}
}
void TraceSenderV1::OpenTraceTimeOffsetFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"-offset.txt";
	m_timeOffset.open(path.c_str(), std::fstream::out);
}
void TraceSenderV1::OnTimeOffset(uint32_t packet_id,uint32_t time_offset){
	if(m_timeOffset.is_open()){
		char line [256];
		memset(line,0,256);
		sprintf(line, "%d %16d",packet_id,time_offset);
		m_timeOffset<<line<<std::endl;
	}
}
void TraceSenderV1::Close(){
	CloseRateFile();
	CloseLossFile();
	CloseSumRateFile();
	CloseTimeOffsetFile();
}
void TraceSenderV1::CloseRateFile(){
	if(m_rate.is_open()){
		m_rate.close();
	}
}
void TraceSenderV1::CloseLossFile(){
	if(m_loss.is_open()){
		m_loss.close();
	}
}
void TraceSenderV1::CloseSumRateFile(){
	if(m_sumrate.is_open()){
		m_sumrate.close();
	}
}
void TraceSenderV1::CloseTimeOffsetFile(){
	if(m_timeOffset.is_open()){
		m_timeOffset.close();
	}
}
TraceReceiverV1::~TraceReceiverV1(){
	Close();
}
void TraceReceiverV1::OpenTraceRecvFrameFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"-recv.txt";
	m_recvFrame.open(path.c_str(), std::fstream::out);
}
void TraceReceiverV1::OnRecvFrame(uint32_t fid,uint32_t delay,uint8_t received,uint8_t total){
	if(m_recvFrame.is_open()){
        float now=Simulator::Now().GetSeconds();
		char line [256];
		memset(line,0,256);
		sprintf(line, "%f %16d %16d %16d %16d",now,fid,delay,received,total);
		m_recvFrame<<line<<std::endl;
	}
}
void TraceReceiverV1::Close(){
	CloseRecvFrameFile();
}
void TraceReceiverV1::CloseRecvFrameFile(){
	if(m_recvFrame.is_open()){
		m_recvFrame.close();
	}
}
TraceState::TraceState(){}
TraceState::~TraceState(){
	Close();
}
void TraceState::OpenTraceStateFile(std::string filename){
	char buf[FILENAME_MAX];
	memset(buf,0,FILENAME_MAX);
	std::string path = std::string (getcwd(buf, FILENAME_MAX)) + "/traces/"
			+ filename+"-state.txt";
	m_state.open(path.c_str(), std::fstream::out);
}
void TraceState::OnNewState(uint64_t bps,std::string state){
	if(m_state.is_open()){
        float now=Simulator::Now().GetSeconds();
        float rate=(float)(bps)/1000;
		char line [256];
		memset(line,0,256);
		sprintf(line, "%f %16f %s",now,rate,state.c_str());
		m_state<<line<<std::endl;
	}
}
void TraceState::Close(){
	CloseStateFile();
}
void TraceState::CloseStateFile(){
	if(m_state.is_open()){
		m_state.close();
	}
}
}




