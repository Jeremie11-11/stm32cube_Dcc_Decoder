/*
 * dcc_signal.h
 *
 *  Created on: Jan 22, 2026
 *      Author: jerem
 */

#ifndef INC_DCC_SIGNAL_H_
#define INC_DCC_SIGNAL_H_


#include "main.h"


enum{
	TIMEOUT__THRES_FREE 		= 232,
	TIMEOUT__THRES_NOCHANGE = 348,
	TIMEOUT__THRES_90KMH 		= 464,
	TIMEOUT__THRES_60KMH 		= 580,
	TIMEOUT__THRES_40KMH 		= 696,
	TIMEOUT__THRES_STOP 		= 812
};


typedef struct{
	uint16_t free_cnt;
	uint16_t nochange_cnt;
	uint16_t sp90kmh_cnt;
	uint16_t sp60kmh_cnt;
	uint16_t sp40kmh_cnt;
	uint16_t stop_cnt;

	uint16_t timeout_tab[8];
	uint32_t in_idx;
	uint32_t out_idx;
} DCC_SIGNAL_STRUCT;


void signal_speed_limit_update(void);


#endif /* INC_DCC_SIGNAL_H_ */
