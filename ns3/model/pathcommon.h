/*
 * pathcommon.h
 *
 *  Created on: 2018��11��1��
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
#define PING_INTERVAL 150
}



#endif /* MULTIPATH_PATHCOMMON_H_ */
