#ifndef NS3_MPVIDEO_WRR_H_
#define NS3_MPVIDEO_WRR_H_
/*this implementation is reference from
 *https://blog.csdn.net/jasonliuvip/article/details/25725541
*/
#include <vector>
#include <stdint.h>
namespace ns3{
struct ServerInfo{
    ServerInfo(int i,uint32_t w):id(i),weight(w){}
	int id;
	uint32_t weight;
};
class WeightRoundRobin{
public:
	WeightRoundRobin():max_weight_(0),cw_(0),i_(-1){}
	~WeightRoundRobin(){}
	void AddServer(int id,uint32_t weight);
	int GetServer();
private:
	int GetGCD();
	uint32_t GetMaxWeight();
private:
std::vector<ServerInfo> servers_;
uint32_t max_weight_;
int cw_;
int i_;
};
}
#endif /* NS3_MPVIDEO_WRR_H_ */
