#include <stdio.h>
#include <inttypes.h>   // for PRId16 (portable int16_t formatting)
#include <stdint.h>
#include <math.h>

#include "fms_magnetometer.h"
#include "main.h"
#include "stm32f411e_discovery_gyroscope.h"



//#define IDLE  0
//#define ON    1
//#define FETCH 2

#define NUM_SAMPLES 200
#define SAMPLE_RATE_MS 10 // 100hz = 10ms
#define gravity 9.81
// TH para comparación y detección de movimiento
#define TH_MAX 20000 // Prueba y error, Calibración + Debug se ha sacado bastante rápido

// Umbrales de comparación
#define TH_NORMAL 50 // < 50 Situación Normal
#define TH_HIGH 100 // 50 < x < 100 Actividad Alta
#define TH_EXTREME 100 // > 100 Actividad Extrema

static uint8_t movement_counter = 0;

// Represents the system state: 1 = active, 0 = inactive
extern volatile uint8_t system_status;

static uint32_t last_time = 0;
static uint8_t samples = 0;
static float pDataXYZ[3];
/**
  * @brief  Get XYZ axes acceleration.
  * @param  pDataXYZ: Pointer to 3 angular acceleration axes.
  *                   pDataXYZ[0] = X axis, pDataXYZ[1] = Y axis, pDataXYZ[2] = Z axis
  */

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

static int is_sample_time (fsm_t* this) {
	uint32_t now = HAL_GetTick();
	if ((now - last_time) >= SAMPLE_RATE_MS) {
		last_time = now;
		return 1;
	} else {
		return 0;
	}
}

static int has_enough_samples(fsm_t* this) {
	if (samples == NUM_SAMPLES) {
		return 1;
	} else {
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////
///		ACTION FUNCTIONS WHEN TRANSITIONS
/////////////////////////////////////////////////////////////////////////
static void fetch_data(fsm_t* this) {
	BSP_GYRO_GetXYZ(pDataXYZ);
	uint32_t movimiento = sqrt(pDataXYZ[0]*pDataXYZ[0] + pDataXYZ[1]*pDataXYZ[1] + pDataXYZ[2]*pDataXYZ[2]) - gravity;

	if (movimiento > TH_MAX){
		movement_counter++;
	}
	samples++;
}

static void generate_results(fsm_t* this) {
	samples = 0;
	if(movement_counter <= TH_NORMAL ){
		HAL_GPIO_WritePin(GPIOD, Green_Led_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, Orange_Led_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, Red_Led_Pin, GPIO_PIN_RESET);
	}
	else if(TH_NORMAL < movement_counter && movement_counter <= TH_HIGH){
		HAL_GPIO_WritePin(GPIOD, Green_Led_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, Orange_Led_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, Red_Led_Pin, GPIO_PIN_RESET);
	}else{
		HAL_GPIO_WritePin(GPIOD, Green_Led_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, Orange_Led_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, Red_Led_Pin, GPIO_PIN_SET);
	}
	movement_counter = 0;

}

static void leds_off(fsm_t* this) {
	HAL_GPIO_WritePin(GPIOD, Red_Led_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, Orange_Led_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, Green_Led_Pin, GPIO_PIN_RESET);
}




fsm_trans_t fsm_magnetometer_tt[] = {
	{ DATA_OFF, check_on, DATA_ON, NULL },
	{ DATA_ON, is_sample_time, DATA_ON, fetch_data },
	{ DATA_ON, has_enough_samples, DATA_ON, generate_results },
	{ DATA_ON, check_off, DATA_OFF, leds_off},
	{ -1, NULL, -1, NULL },
  };

fsm_t* fsm_magnetometer_new(void) {
  return fsm_new(fsm_magnetometer_tt);
}

