/*
 * CameraToSRAM.h
 *
 *  Created on: 9 déc. 2014
 *      Author: Nirgal
 */

#ifndef __CAMERA_H_
#define __CAMERA_H_

#include "macro_types.h"
#include "stm32f4xx_hal.h"

typedef enum
{
	MODE_CAMERA_TO_SRAM,
	MODE_CAMERA_TO_LCD
}CAMERA_mode_e;

uint8_t DCMI_SingleRandomWrite(uint8_t Device, uint16_t Addr, uint8_t Data);
uint8_t DCMI_SingleRandomRead(uint8_t Device, uint16_t Addr);

running_e Camera_statemachine (bool_e ask_for_finish, CAMERA_mode_e mode);
void Camera_reset(void);

void Camera_process_10ms(void);

void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi);




#endif /* CAMERA_H_ */
