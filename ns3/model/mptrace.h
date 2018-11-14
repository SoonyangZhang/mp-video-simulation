#ifndef NS3_MPVIDEO_MPTRACE_H_
#define NS3_MPVIDEO_MPTRACE_H_
#include <iostream>
#include <fstream>
#include <string>
namespace ns3{
class Mptrace{
public:
	Mptrace(){}
	~Mptrace();
	void OpenTraceFile(std::string filename);
	void CloseTraceFile();
	void FrameRecvGap(uint32_t fid,uint32_t duration);
private:
	void Close();
	std::fstream m_frameGap;
};
class TraceDelayInfo{
public:
	TraceDelayInfo(){}
	~TraceDelayInfo();
	//trace packet waiting delay at sender buffer.
	void OpenTracePendingDelayFile(std::string filename);
	//trace owe way delay
	void OpenTraceOwdFile(std::string filename);
	void OpenTraceRateFile(std::string filename);
	void ClosePendingDelayFile();
	void CloseOwdFile();
	void CloseRateFile();
	void RecvOwd(uint32_t packet_id,uint32_t owd);
	void RecvPendDelay(uint32_t packet_id,uint32_t sent_ts);
	void RecvRate(uint32_t bps);
private:
	void Close();
	std::fstream m_owd;
	std::fstream m_pending;
	std::fstream m_rate;
};
}
#endif /* NS3_MPVIDEO_MPTRACE_H_ */
