#ifndef FSM_BLINK_H
#define FSM_BLINK_H

#include "fsm.h"
#include "main.h"
#include <stdint.h>

//======================================
// External variables
//======================================
	extern fsm_trans_t fsm_system_led_tt[];
	fsm_t* fsm_system_led_new(void);

#endif /* FSM_BLINK_H */
