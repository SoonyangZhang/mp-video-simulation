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
}




#endif /* NS3_MPVIDEO_MPTRACE_H_ */
