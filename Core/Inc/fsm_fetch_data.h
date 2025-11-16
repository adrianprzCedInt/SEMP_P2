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

//int is_system_on(fsm_t* this);
//int is_idle(fsm_t* this);
int is_sample_time(fsm_t* this);
int has_enough_samples(fsm_t* this);

//void fetch_data (fsm_t* this);
//void generate_results(fsm_t* this);
//void leds_off (fsm_t* this);


#endif /* INC_FSM_FETCH_DATA_H_ */
