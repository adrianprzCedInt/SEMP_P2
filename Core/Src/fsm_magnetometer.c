#include <stdio.h>
#include <inttypes.h>   // for PRId16 (portable int16_t formatting)
#include <stdint.h>
#include <math.h>

#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "arm_math.h"
#include "main.h"
#include "fsm_magnetometer.h"
#include "lsm303_mag.h"
#include "stm32f411e_discovery_gyroscope.h"
#include "cmsis_os.h"


#define DATA_OFF	0
#define DATA_ON		1

#define NUM_SAMPLES 200
#define SAMPLE_RATE_MS 10 // 100hz = 10ms

// Represents the system state: 1 = active, 0 = inactive
extern volatile uint8_t system_status;

static uint32_t last_time = 0;
static uint8_t samples = 0;
static int16_t magnetometerDataXYZ[3];

static float32_t max_diff = 0;
static float32_t max_diffX = 0;
static float32_t max_diffY = 0;
static float32_t max_diffZ = 0;
static float32_t abs_mag;


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
	LSM303AGR_MagReadXYZ(magnetometerDataXYZ);
	osSemaphoreRelease(i2c_semHandle);
	}

	float32_t sqrt = magnetometerDataXYZ[0]*magnetometerDataXYZ[0] + magnetometerDataXYZ[1]*magnetometerDataXYZ[1] + magnetometerDataXYZ[2]*magnetometerDataXYZ[2];
	if (arm_sqrt_f32(sqrt, &abs_mag) == ARM_MATH_SUCCESS){
		abs_mag = abs_mag;
	}
	if(abs_mag > max_diff){
		max_diff = abs_mag;
		max_diffX = magnetometerDataXYZ[0];
		max_diffY = magnetometerDataXYZ[1];
		max_diffZ = magnetometerDataXYZ[2];
	}
	samples++;
}

static void generate_msg(fsm_t* this) {
	samples = 0;
	printf("[MAG] Maximo detectado: %d [X,Y,Z]: [%d,%d,%d]\r\n", (int)max_diff, (int)magnetometerDataXYZ[0], (int)magnetometerDataXYZ[1], (int)magnetometerDataXYZ[2]);
	printf("-------------------------------------------------------\r\n");

	max_diff = 0;
}

static void leds_off(fsm_t* this) {
	HAL_GPIO_WritePin(GPIOD, Red_Led_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, Orange_Led_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, Green_Led_Pin, GPIO_PIN_RESET);
}


fsm_trans_t fsm_magnetometer_tt[] = {
	{ DATA_OFF, check_on, DATA_ON, NULL },
	{ DATA_ON, has_enough_samples, DATA_ON, generate_msg },
	{ DATA_ON, is_sample_time, DATA_ON, fetch_data },
	{ DATA_ON, check_off, DATA_OFF, leds_off},
	{ -1, NULL, -1, NULL },
  };

fsm_t* fsm_magnetometer_new(void) {
  return fsm_new(fsm_magnetometer_tt);
}

void fsm_system_mag_task(void* arguments) {

	fsm_t* fs = fsm_magnetometer_new();
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
