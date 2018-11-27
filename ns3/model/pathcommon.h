/*
 * pathcommon.h
 *
 *  Created on: 2018Äê11ÔÂ1ÈÕ
 *      Author: LENOVO
 */

#ifndef MULTIPATH_PATHCOMMON_H_
#define MULTIPATH_PATHCOMMON_H_

#include "mpcommon.h"
#include "sim_proto.h"
namespace ns3{
struct send_buf_t{
	uint32_t ts;
	sim_segment_t *seg;
};
enum PaceMode{
	no_congestion,// totally know the path bandwidth
	with_congestion, //work with GCC
};
#define PING_INTERVAL 150
}



#endif /* MULTIPATH_PATHCOMMON_H_ */
