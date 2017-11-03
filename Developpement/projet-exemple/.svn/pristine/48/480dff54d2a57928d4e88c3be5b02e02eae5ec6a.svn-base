/**
 * demo_dac.c
 *
 *  Created on: 3 mars 2015
 *      Author:
 */
#include "demo_rng.h"
#include "demo_touchscreen.h"
#include "lcd_display_form_and_text.h"
#include "stm32f4xx_hal.h"
#include "stm32f4_gpio.h"


static volatile uint16_t t = 0;
void RNG_init_and_configure(void);
RNG_HandleTypeDef RNG_HandleStructure;

/**
 * @brief 	Cette fonction est une machine a etat qui regit le comportement de la demo du rng.
 * @func 	running_e DEMO_rng_statemachine (bool_e ask_for_finish)
 * @param 	exit_asked: demande a l'application de quitter ou non 
 * @return	cette fonction retourne un element de l'enumeration running_e (END_OK= l'application est quittee avec succes ou IN_PROGRESS= l'application est toujours en cours)
 */
running_e DEMO_rng_statemachine (bool_e ask_for_finish)
{
	typedef enum
	{
		INIT = 0,
		DISPLAY,
		ADJUST_RNG,
		CLOSE
	}state_e;
	running_e ret = IN_PROGRESS;
	static state_e state = INIT;
	switch(state)
	{
		case INIT:
			RNG_init_and_configure(); //Configuration et initialisation du rng
			state = DISPLAY;
			break;
		case DISPLAY:
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_DrawRect(80-1,40-1,200+2,50+2,LCD_COLOR_RED);
			LCD_SetFont(&Font8x8);
			LCD_DisplayStringLine(LINE(18),COLUMN(0),(uint8_t *)"      Useless Random Number Generator",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(21),COLUMN(0),(uint8_t *)"           Every 1 sec",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_SetFont(&Font12x12);
			state = ADJUST_RNG;
			break;
		case ADJUST_RNG:{	//L'accolade ici permet de definir des variables locales juste en dessous, c'est un peu moche, mais assez pratique.
			uint32_t value;
			char text[10];
			if(!t)
			{
				t = 1000;
				if(HAL_RNG_GenerateRandomNumber(&RNG_HandleStructure, &value) != HAL_OK) //On genere un nombre aléatoire
				{
					// Erreur a gerer
				}
				value = (uint32_t)((unsigned int)value % 100); //on le convertit en pourcentage...
				sprintf(text,"%3u%%",(unsigned int)value);		//...que l'on affiche en chiffre ...
				LCD_DisplayStringLine(LINE(5),COLUMN(0),(uint8_t *)text,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				if(value > 0)
					LCD_DrawFullRect(80,40,(value*2),50,LCD_COLOR_RED,LCD_COLOR_GREEN); //... et avec une jauge
				if(value < 100)
					LCD_DrawFullRect(80+(value*2),40,((100-value)*2),50,LCD_COLOR_RED,LCD_COLOR_WHITE);
			}
			if(ask_for_finish)
				state = CLOSE;
			break;}
		case CLOSE:
			__HAL_RNG_DISABLE(&RNG_HandleStructure);
			state = INIT;
			ret = END_OK;
			break;
		default:
			break;
	}

	return ret;
}

void DEMO_rng_process_1ms(void)
{
	if(t)
		t--;
}




/**
 * @brief	Cette fonction initialise la broche PA5 comme analogique et active le DAC.
 *
 */
void RNG_init_and_configure(void)
{
	// Set the RNG clock active
	__HAL_RCC_RNG_CLK_ENABLE();

	RNG_HandleStructure.Instance = RNG;
	HAL_RNG_Init(&RNG_HandleStructure);
	// Enable the RNG peripheral
	__HAL_RNG_ENABLE(&RNG_HandleStructure);

}
