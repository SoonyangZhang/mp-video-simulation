#ifndef NS3_MPVIDEO_TRACE_V1_H_
#define NS3_MPVIDEO_TRACE_V1_H_
#include <string>
#include <iostream>
#include <fstream>
namespace ns3{
class TraceSenderV1{
public:
	TraceSenderV1(){}
	~TraceSenderV1();
	void OpenTraceRateFile(std::string filename);
	void OnRate(uint32_t bps);
	void OpenTraceLossFile(std::string filename);
	void OnLoss(uint32_t seq,uint32_t rtt);
	void OpenTraceSumRateFile(std::string filename);
	void OnSumRate(uint32_t bps);
	void OpenTraceTimeOffsetFile(std::string filename);
	void OnTimeOffset(uint32_t packet_id,uint32_t time_offset);
private:
	void Close();
	void CloseRateFile();
	void CloseLossFile();
	void CloseSumRateFile();
	void CloseTimeOffsetFile();
	std::fstream m_rate;
	std::fstream m_loss;
	std::fstream m_sumrate;
	std::fstream m_timeOffset;
};
class TraceReceiverV1{
public:
	TraceReceiverV1(){}
	~TraceReceiverV1();
	void OpenTraceRecvFrameFile(std::string filename);
	void OnRecvFrame(uint32_t fid,uint32_t delay,uint8_t received,uint8_t total);
	void OpenTraceOwdFile(std::string filename);
	void OnRecvOwd(uint64_t seq,uint32_t owd);
private:
	void Close();
	void CloseRecvFrameFile();
	void CloseOwdFile();
	std::fstream m_recvFrame;
	std::fstream m_owd;
};
class TraceState{
public:
	TraceState();
	~TraceState();
	void OpenTraceStateFile(std::string filename);
	void OnNewState(uint64_t bps,std::string state);
private:
	void Close();
	void CloseStateFile();
	std::fstream m_state;
};
}
#endif /* NS3_MPVIDEO_TRACE_V1_H_ */
