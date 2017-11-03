/**
 * demo_adc.h
 *
 *  Created on: 3 mars 2015
 *      Author: Nirgal
 */

#ifndef DEMO_ADC_H_
#define DEMO_ADC_H_

#include "macro_types.h"
#include <stdint.h>

running_e DEMO_adc_statemachine (bool_e ask_for_finish);
void DEMO_adc_process_1ms(void);


#endif /* DEMO_ADC_H_ */

