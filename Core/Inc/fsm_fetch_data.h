/*
 * fsm_fetch_data.h
 *
 *  Created on: Oct 22, 2025
 *      Author: DELL
 */

#ifndef INC_FSM_FETCH_DATA_H_
#define INC_FSM_FETCH_DATA_H_

#include "fsm.h"

extern fsm_trans_t fsm_data_fetch_tt[];
fsm_t* fsm_fetch_data_new(void);
void fsm_system_acc_task(void* arguments);
extern osSemaphoreId_t i2c_semHandle;
extern osSemaphoreId_t printf_semHandle;



#endif /* INC_FSM_FETCH_DATA_H_ */
