/**
 * demo_dac.h
 *
 *  Created on: 3 mars 2015
 *      Author: Nirgal
 */

#ifndef DEMO_DAC_H_
#define DEMO_DAC_H_

#include "macro_types.h"
#include <stdint.h>

running_e DEMO_dac_statemachine (bool_e ask_for_finish, char touch_pressed);
void DEMO_dac_process_1ms(void);
void DAC_set_value(uint8_t value);

#endif /* DEMO_DAC_H_ */
