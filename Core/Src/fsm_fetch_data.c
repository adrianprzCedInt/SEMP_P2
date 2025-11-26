#include <stdio.h>
#include <inttypes.h>   // for PRId16 (portable int16_t formatting)
#include <stdint.h>
#include <math.h>

#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
//#include "freertos.c"
#include "arm_math.h"
#include "fsm_fetch_data.h"
#include "main.h"
#include "stm32f411e_discovery_accelerometer.h"
#include "cmsis_os.h"

#define DATA_OFF	0
#define DATA_ON		1

#define NUM_SAMPLES 200
#define SAMPLE_RATE_MS 10 // 100hz = 10ms

// TH en mg para comparación y detección de movimiento
#define TH_MAX 400
// Contante para pasar de LSB a mg segun datasheet
#define LSB2mg 16129//Sensibilidad magnetometro segun ds 0.062mg/LSB / 1/0.062= 16129

// Umbrales de comparación
#define TH_NORMAL 50 // < 50 Situación Normal
#define TH_HIGH 100 // 50 < x < 100 Actividad Alta
#define TH_EXTREME 100 // > 100 Actividad Extrema

static uint8_t movement_counter = 0;
static float32_t max_diff = 0;
static float32_t min_diff = 100000000;

// Represents the system state: 1 = active, 0 = inactive
extern volatile uint8_t system_status;

static uint32_t last_time = 0;
static uint8_t samples = 0;
static int16_t pDataXYZ[3];
static float32_t abs_acc;
/**
  * @brief  Get XYZ axes acceleration.
  * @param  pDataXYZ: Pointer to 3 angular acceleration axes.
  *                   pDataXYZ[0] = X axis, pDataXYZ[1] = Y axis, pDataXYZ[2] = Z axis
  */

/////////////////////////////////////////////////////////////////////////
///		CHECKED FUNCTIONS FOR TRANSITIONS
/////////////////////////////////////////////////////////////////////////
/*
int is_system_on (fsm_t* this) {
	return system_status;
}

int is_idle (fsm_t* this) {
	return !system_status;
}
*/
// Check transition function true (1) if system is on
static int check_on (fsm_t* this)	{
	return system_status;
}


// Check transition function true (1) if system is off (!system_status)
static int check_off (fsm_t* this)	{
	return !system_status;
}

static int is_sample_time (fsm_t* this) {
	//uint32_t now = HAL_GetTick();
	uint32_t now = osKernelGetTickCount();
	if ((now - last_time) >= SAMPLE_RATE_MS) {
		last_time = now;
		return 1;
	} else {
		return 0;
	}
}

static int has_enough_samples(fsm_t* this) {
	if (samples >= NUM_SAMPLES) {
		return 1;
	} else {
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////
///		ACTION FUNCTIONS WHEN TRANSITIONS
/////////////////////////////////////////////////////////////////////////
static void fetch_data(fsm_t* this) {
	if(osSemaphoreAcquire(i2c_semHandle, 5) == osOK){
		BSP_ACCELERO_GetXYZ(pDataXYZ);
		osSemaphoreRelease(i2c_semHandle);
	}


	float32_t sqrt = pDataXYZ[0]*pDataXYZ[0] + pDataXYZ[1]*pDataXYZ[1] + pDataXYZ[2]*pDataXYZ[2];
	if (arm_sqrt_f32(sqrt, &abs_acc) == ARM_MATH_SUCCESS){
		abs_acc = (abs_acc/LSB2mg)*1000 -1000;
	}

	if (abs_acc > max_diff){
		max_diff = abs_acc;
	}
	if (abs_acc < min_diff){
		min_diff = abs_acc;
	}

	if (abs_acc > TH_MAX){
		movement_counter++;
	}
	samples++;
}

static void generate_results(fsm_t* this) {
	samples = 0;
	if(osSemaphoreAcquire(printf_semHandle, 5) == osOK){
	//printf("[ACC] Maxima diferencia: %d, Minima diferencia: %d, Numero detecciones: %d\r\n", (int)max_diff, (int)min_diff, (int)movement_counter);
	printf("-------------------------------------------------------\r\n");
	printf("[ACC] Maxima diferencia: %d\r\n", (int)max_diff);
	printf("[ACC] Minima diferencia: %d\r\n", (int)min_diff);
	printf("[ACC] Numero detecciones: %d\r\n", (int)movement_counter);
	osSemaphoreRelease(printf_semHandle);
	}
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


fsm_trans_t fsm_data_fetch_tt[] = {
	{ DATA_OFF, check_on, DATA_ON, NULL },
	{ DATA_ON, has_enough_samples, DATA_ON, generate_results },
	{ DATA_ON, is_sample_time, DATA_ON, fetch_data },
	{ DATA_ON, check_off, DATA_OFF, leds_off},
	{ -1, NULL, -1, NULL },
  };

fsm_t* fsm_fetch_data_new(void) {
  return fsm_new(fsm_data_fetch_tt);
}

void fsm_system_acc_task(void* arguments) {

	fsm_t* fs = fsm_fetch_data_new();
	const uint32_t period = SAMPLE_RATE_MS;
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

