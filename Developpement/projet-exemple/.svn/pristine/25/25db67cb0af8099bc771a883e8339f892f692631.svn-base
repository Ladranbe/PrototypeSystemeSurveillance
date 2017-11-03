/*
 * demo_accelerometer.c
 *
 *  Created on: 3 mars 2015
 *      Author: Nirgal
 */
#include "demo_accelerometer.h"
#include "stm32f4_accelerometer.h"
#include "lcd_display_form_and_text.h"
#include "demo_touchscreen.h"
#include "stm32f4_gpio.h"
#include "stm32f4xx_hal.h"


static volatile uint8_t t = 0;

/**
 * @brief 	Cette fonction est une machine a etat qui regit le comportement de la demo de l'accelerometre.
 * @func 	running_e DEMO_accelerometer_state_machine(bool_e exit_asked)
 * @param 	exit_asked: demande a l'application de quitter ou non 
 * @return	cette fonction retourne un element de l'enumeration running_e (END_OK= l'application est quittee avec succes ou IN_PROGRESS= l'application est toujours en cours)
 */
running_e DEMO_accelerometer_state_machine(bool_e exit_asked)
{
	typedef enum
	{
		INIT = 0,
		WAIT_LCD_READY,
		DISPLAY,
		CLOSE
	}state_e;

	typedef enum{
		DISPLAY_MODE_SHOW_VALUES = 0,
		DISPLAY_MODE_SQUARE,
		DISPLAY_MODE_CLOUD,
		DISPLAY_MODE_NUMBER
	}display_mode_e;
	
	static state_e state = INIT;
	running_e ret= IN_PROGRESS;
	int16_t xyz[3];
	static int16_t previous_x, previous_y;
	static display_mode_e display_mode = DISPLAY_MODE_SHOW_VALUES;
	char text[30];

	switch(state)
	{
		case INIT:
			t = 2;
			state = WAIT_LCD_READY;
			display_mode = DISPLAY_MODE_SHOW_VALUES;
			//ATTENTION : avant d'initialiser l'accelero, il faut attendre que l'ecran ne soit plus en train de recevoir des donnees du FSMC.
			//				ceci est lie au conflit du port E3, (utilise sur le LCD et l'accelero)
			break;
		case WAIT_LCD_READY:
			if(!t)
			{
				DEMO_accelerometer_open();
				LCD_SetFont(&Font8x8);
				LCD_DisplayStringLine(LINE(23),COLUMN(0),(uint8_t *)"Press screen to change display mode",LCD_COLOR_RED,LCD_COLOR_BLACK,LCD_NO_DISPLAY_ON_UART);
				state = DISPLAY;
			}
			break;

		case DISPLAY:{
			int16_t x,y;
			bool_e current_touch;
			static bool_e previous_touch = FALSE;

			if(!t)
			{
				t = 50;

				current_touch = TS_Get_Touch((uint16_t *)&x,(uint16_t *)&y); //On lit un appuie sur l'ecran tactile
				if(current_touch && !previous_touch)
				{
					LCD_Clear(LCD_COLOR_BLACK);
					display_mode = (display_mode+1 < DISPLAY_MODE_NUMBER)?display_mode+1:0;
				}
				previous_touch = current_touch;

				DEMO_ACCELERO_SetPorts(PORTS_FOR_ACCELERO);		//on configure les ports pour pouvoir lire les donnees dans l'accelerometre...
				BSP_ACCELERO_GetXYZ(xyz);						//...on les lit...
				DEMO_ACCELERO_SetPorts(PORTS_FOR_LCD);			//...et on reconfigure les ports pour l'utilisation de l'ecran
				
				/*On divise x par 2^3 = 8  ce qui est equivalent a un reglage de sensibilite*/
				x = xyz[0] >> 3;
				/*On garde le x dans les limites de l'ecran*/
				x = MAX(-160,x);
				x = MIN(160,x);
				
				/*On divise y par 2^3 = 8*/
				y = xyz[1] >> 3;
				/*On garde le y dans les limites de l'ecran*/
				y = MAX(-120,y);
				y = MIN(120,y);

				switch(display_mode)
				{
					case DISPLAY_MODE_SHOW_VALUES:
						LCD_SetFont(&Font16x24);
						sprintf(text,"x:%5d ",xyz[0]);
						LCD_DisplayStringLine(LINE(0),COLUMN(0),(uint8_t *)text,LCD_COLOR_WHITE,LCD_COLOR_BLACK,LCD_NO_DISPLAY_ON_UART);
						printf(text);
						sprintf(text,"y:%5d ",xyz[1]);
						LCD_DisplayStringLine(LINE(1),COLUMN(0),(uint8_t *)text,LCD_COLOR_WHITE,LCD_COLOR_BLACK,LCD_NO_DISPLAY_ON_UART);
						printf(text);
						sprintf(text,"z:%5d ",xyz[2]);
						LCD_DisplayStringLine(LINE(2),COLUMN(0),(uint8_t *)text,LCD_COLOR_WHITE,LCD_COLOR_BLACK,LCD_NO_DISPLAY_ON_UART);
						printf(text);
						printf("\n");
						LCD_PutPixel(previous_x+160,previous_y+120,LCD_COLOR_BLACK);
						LCD_PutPixel(x+160,y+120,LCD_COLOR_WHITE);
						break;
					case DISPLAY_MODE_SQUARE:
						/* les +120 et +160 visent à centrer le carre sur l'ecran (width/2 et height/2)*/
						LCD_DrawRect(previous_x+160	,previous_y+120	,5,	5, 	LCD_COLOR_BLACK);
						LCD_DrawRect(x+160			,y+120			,5, 5,	LCD_COLOR_YELLOW);
						break;
					case DISPLAY_MODE_CLOUD:
						LCD_PutPixel(x+160,y+120,xyz[2]<<6);
						break;
					default:
						break;
				}



				previous_x = x;
				previous_y = y;
			}

			if(exit_asked)
				state = CLOSE;
			break;}
		case CLOSE:
			DEMO_accelerometer_close();
			state = INIT;
			ret = END_OK;
			break;
		default:
			break;
	}

	return ret;
}

/** @brief 	fonction appelee par la routine d'interruption du timer toutes les ms
 * @func	void DEMO_accelerometer_process_1ms(void)
 */
void DEMO_accelerometer_process_1ms(void)
{
	if(t)
		t--;
}

void DEMO_accelerometer_open(void)
{
	DEMO_ACCELERO_SetPorts(PORTS_FOR_ACCELERO);
	BSP_ACCELERO_Init();
	BSP_ACCELERO_output_on_opendrain();
	//BSP_ACCELERO_WakeUp();
	DEMO_ACCELERO_SetPorts(PORTS_FOR_LCD);
}

void DEMO_accelerometer_close(void)
{
	DEMO_ACCELERO_SetPorts(PORTS_FOR_ACCELERO);
	BSP_ACCELERO_Init();
	BSP_ACCELERO_output_on_opendrain();
	BSP_ACCELERO_DeInit();
	DEMO_ACCELERO_SetPorts(PORTS_FOR_LCD);
}


/** @brief	Configure les ports du GPIO pour l'ecran LCD ou l'accelerometre
 * @func	void DEMO_ACCELERO_SetPorts(accelero_or_lcd_e accelero_or_lcd)
 * @param 	accelero_or_lcd element de l'enumeration accelero_or_lcd_e(peut etre PORTS_FOR_ACCELERO ou PORTS_FOR_LCD)
*/
void DEMO_ACCELERO_SetPorts(accelero_or_lcd_e accelero_or_lcd)
{
	if(accelero_or_lcd == PORTS_FOR_ACCELERO)
	{
		BSP_GPIO_PinCfg(GPIOA, (GPIO_PIN_6 | GPIO_PIN_7), GPIO_MODE_AF_PP, GPIO_PULLDOWN, GPIO_SPEED_MEDIUM, DISCOVERY_SPIx_AF);
		BSP_GPIO_PinCfg(GPIOE, GPIO_PIN_3, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FAST, 0);
	}
	else
	{
		BSP_GPIO_PinCfg(GPIOA, GPIO_PIN_6, GPIO_MODE_AF_PP, GPIO_PULLDOWN, GPIO_SPEED_MEDIUM, GPIO_AF13_DCMI);
		BSP_GPIO_PinCfg(GPIOE, GPIO_PIN_3, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FAST, GPIO_AF12_FSMC);
	}
}
