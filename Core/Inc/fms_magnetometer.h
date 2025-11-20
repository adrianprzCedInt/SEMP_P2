/*
 * fms_magnetometer.h
 *
 *  Created on: Nov 20, 2025
 *      Author: adria
 */

#ifndef INC_FMS_MAGNETOMETER_H_
#define INC_FMS_MAGNETOMETER_H_

#include "fsm.h"

enum { DATA_OFF = 0, DATA_ON = 1 };
extern fsm_trans_t fsm_magnetometer_tt[];
fsm_t* fsm_magnetometer_new(void);

#endif /* INC_FMS_MAGNETOMETER_H_ */
