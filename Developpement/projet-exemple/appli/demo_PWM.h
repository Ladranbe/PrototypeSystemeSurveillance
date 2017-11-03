/**
 * demo_pwm.h
 *
 *  Created on: 3 mars 2015
 *      Author: Nirgal
 */

#ifndef DEMO_PWM_H_
#define DEMO_PWM_H_

#include "macro_types.h"

running_e DEMO_pwm_statemachine (bool_e ask_for_finish, char touch_pressed);
void DEMO_pwm_process_1ms(void);


#endif /* DEMO_PWM_H_ */
