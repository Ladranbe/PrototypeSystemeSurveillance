/**
 * demo_accelerometer.h
 *
 *  Created on: 3 mars 2015
 *      Author: Nirgal
 */

#ifndef DEMO_ACCELEROMETER_H_
#define DEMO_ACCELEROMETER_H_

#include "macro_types.h"

typedef enum
{
	PORTS_FOR_ACCELERO,
	PORTS_FOR_LCD
}accelero_or_lcd_e;

running_e DEMO_accelerometer_state_machine(bool_e exit_asked);
void DEMO_accelerometer_process_1ms(void);
void DEMO_ACCELERO_SetPorts(accelero_or_lcd_e accelero_or_lcd);

void DEMO_accelerometer_open(void);
void DEMO_accelerometer_close(void);

#endif /* DEMO_ACCELEROMETER_H_ */
