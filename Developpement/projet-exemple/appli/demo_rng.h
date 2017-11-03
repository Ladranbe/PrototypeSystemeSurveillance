/*
 * demo_dac.h
 *
 *  Created on: 3 mars 2015
 *      Author:
 */

#ifndef DEMO_RNG_H_
#define DEMO_RNG_H_

#include "macro_types.h"

running_e DEMO_rng_statemachine (bool_e ask_for_finish);
void DEMO_rng_process_1ms(void);

#endif /* DEMO_RNG_H_ */
