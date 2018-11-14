#ifndef NS3_MPVIDEO_FUN_TEST_T_TRACE_H_
#define NS3_MPVIDEO_FUN_TEST_T_TRACE_H_
#include <iostream>
#include <fstream>
#include <string>
namespace ns3{
class TTrace{
public:
	TTrace(){}
	~TTrace();
	void OpenTraceRateFile(std::string n);
	void CloseRateFile();
	void OpenTracePendingLenFile(std::string n);
	void ClosePengingLen();
	void RecvPendingLen(uint32_t len);
	void RecvRate(uint32_t bps);
private:
	void Close();
	std::fstream m_rate;
	std::fstream m_pending_len;
};
}




#endif /* NS3_MPVIDEO_FUN_TEST_T_TRACE_H_ */
