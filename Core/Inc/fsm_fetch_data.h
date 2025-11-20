/*
 * fsm_fetch_data.h
 *
 *  Created on: Oct 22, 2025
 *      Author: DELL
 */

#ifndef INC_FSM_FETCH_DATA_H_
#define INC_FSM_FETCH_DATA_H_

#include "fsm.h"

enum { DATA_OFF = 0, DATA_ON = 1 };
extern fsm_trans_t fsm_data_fetch_tt[];
fsm_t* fsm_fetch_data_new(void);



#endif /* INC_FSM_FETCH_DATA_H_ */
