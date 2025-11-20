/*
 * fms_magnetometer.h
 *
 *  Created on: Nov 20, 2025
 *      Author: adria
 */

#ifndef INC_FSM_MAGNETOMETER_H_
#define INC_FSM_MAGNETOMETER_H_

#include "fsm.h"

extern fsm_trans_t fsm_magnetometer_tt[];
fsm_t* fsm_magnetometer_new(void);

#endif /* INC_FSM_MAGNETOMETER_H_ */
