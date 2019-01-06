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
	void OpenTraceOwdFile(std::string n);
	void CloseOwd();
	void OpenTraceRtsFile(std::string n);
	void CloseRts();
	void RecvPendingLen(uint32_t len);
	void RecvRate(uint32_t bps);
	void RecvOwd(uint16_t seq,uint32_t sts,uint32_t rts,uint32_t ms);
	void RecvRts(uint16_t seq,uint32_t rts);
private:
	void Close();
	std::fstream m_rate;
	std::fstream m_pending_len;
	std::fstream m_owd;
	std::fstream m_rts;
};
}




#endif /* NS3_MPVIDEO_FUN_TEST_T_TRACE_H_ */
