#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include "main.h"
#include "fsm.h"
#include "fsm_system_led.h"
#include "cmsis_os.h"



//======================
// Config
//======================
#define OFF	0
#define ON	1

#define BLINK_PERIOD_MS 1000
#define SLEEP_PERIOD_MS 2000

static uint32_t last_time = 0;

// Declared as extern here, defined in main.c
// Represents the system state: 1 = active, 0 = inactive
extern volatile uint8_t system_status;



/////////////////////////////////////////////////////////////////////////
///		CHECKED FUNCTIONS FOR TRANSITIONS
/////////////////////////////////////////////////////////////////////////

// Check transition function true (1) if system is on
static int check_on (fsm_t* this)	{
	return system_status;
}


// Check transition function true (1) if system is off (!system_status)
static int check_off (fsm_t* this)	{
	return !system_status;
}

// Check transition function, each BLINK_PERIOD_MS the led toggles
static int check_blink_period (fsm_t* this)	{
	  //uint32_t now = HAL_GetTick();
	  uint32_t now = osKernelGetTickCount();
	// Check toogle period
	if ((now - last_time) >= BLINK_PERIOD_MS) {
		last_time = now;
		return 1;
	} else {
		return 0;
	}
}

// Check SLEEP_MODE Period
static int check_sleep (fsm_t* this)	{
	//uint32_t now = HAL_GetTick();
	uint32_t now = osKernelGetTickCount();
	// Check toogle period
	if ((now - last_time) >= SLEEP_PERIOD_MS) {
		last_time = now;
		return 1;
	} else {
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////
///		ACTION FUNCTIONS WHEN TRANSITIONS
/////////////////////////////////////////////////////////////////////////

// Toggles blue led status ON -> OFF, OFF -> ON
static void system_led_on (fsm_t* this) {
	printf("[BUT] SYSTEM ON\r\n");
	HAL_GPIO_WritePin(GPIOD, Blue_Led_Pin, GPIO_PIN_SET);
}


// Toggles blue led status ON -> OFF, OFF -> ON
static void toggle_led (fsm_t* this) {
	HAL_GPIO_TogglePin(GPIOD, Blue_Led_Pin);
}

// Sets all led's status => OFF even if not used, for requirements compliance.
static void led_off (fsm_t* this) {
	printf("[BUT] SYSTEM OFF\r\n");
	HAL_GPIO_WritePin(GPIOD, Blue_Led_Pin, GPIO_PIN_RESET);
}

static void sleep (fsm_t* this) {
	printf("[SYS] Going to sleep mode\r\n");
	//Suspendemos contador de tick
	HAL_SuspendTick();
	//Habilitamos salir con interrupciones
	HAL_PWR_EnableSleepOnExit();
	// CPU se detiene, Perifericos siguen funcionando, Pines GPIO mantienen estado y consumo baja
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

// Explicit FSM description
// {INITIAL_STATE,	CHECKED_FUNCTION, 	NEXT_STATE, 	TRANSITION_FUNCTION}
fsm_trans_t fsm_system_led_tt[] = {
	//{ OFF,      check_sleep, 		OFF,  	sleep 	  	  },
	{ OFF,      check_on, 			ON,  	system_led_on },
	{ ON, 		check_off, 			OFF,    led_off       },
	{ ON, 		check_blink_period, ON,     toggle_led    },
	{-1, NULL, -1, NULL },
  };

fsm_t* fsm_system_led_new(void) {
  return fsm_new(fsm_system_led_tt);
}

void fsm_system_led_task(void* arguments) {

	fsm_t* fs = fsm_system_led_new();
    uint32_t period = 4; // ms → 100 Hz
    uint32_t last_wake = osKernelGetTickCount();

	for (;;) {
		fsm_fire(fs);
        osDelayUntil(last_wake + period);
        last_wake += period;
		//uint32_t next_period = osKernelGetTickCount() + 1;

		//osDelayUntil(next_period);
		//osDelay(3);
	}
}
