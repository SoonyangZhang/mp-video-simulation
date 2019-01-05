/*
 * edcld_v1.h
 *
 *  Created on: 2019Äê1ÔÂ3ÈÕ
 *      Author: LENOVO
 */

#ifndef NS3_MPVIDEO_EDCLD_V1_H_
#define NS3_MPVIDEO_EDCLD_V1_H_
#include "interface_v1.h"
namespace zsy{
class EDCLDScheduleV1:public ScheduleV1{
public:
	EDCLDScheduleV1(float weight);
	~EDCLDScheduleV1(){}
	void IncomingPackets(std::map<uint32_t,uint32_t>&packets,
			uint32_t size) override;
	void RegisterPath(uint8_t pid) override;
	void UnregisterPath(uint8_t pid) override;
private:
	void GetUsablePath(std::map<uint8_t,uint32_t>&usable_path,uint32_t &total_rate);
	void WeightRoundRobinSchedule(std::map<uint32_t,uint32_t>&packets,std::map<uint8_t,double>&lambda);
	void UpdateCostAndPsiTable(uint32_t now,uint32_t len);
	void ResetCostAndPsiTable();
	double w_;
	uint32_t last_ts_;
	std::map<uint8_t,double> cost_table_;
	std::map<uint8_t,double> psi_table_;
	uint32_t average_packet_len_;
	uint64_t packet_counter_;
    double smooth_rate_{0};
    double coeff_{0.85};
};
}
#endif /* NS3_MPVIDEO_EDCLD_V1_H_ */
