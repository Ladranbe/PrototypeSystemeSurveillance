/**
 * demo_dac.c
 *
 *  Created on: 3 mars 2015
 *      Author: Samuel Poiraud
 */
#include "demo_dac.h"
#include "demo_touchscreen.h"
#include "lcd_display_form_and_text.h"
#include "stm32f4_gpio.h"
#include "stm32f4xx_hal.h"


DAC_HandleTypeDef DAC_HandleStructure;
static volatile uint8_t t = 0;
void DAC_init_and_configure(void);

/**
 * @brief 	Cette fonction est une machine a etat qui regit le comportement de la demo du dac.
 * @func 	running_e DEMO_dac_statemachine (bool_e ask_for_finish)
 * @param 	exit_asked: demande a l'application de quitter ou non 
 * @return	cette fonction retourne un element de l'enumeration running_e (END_OK= l'application est quittee avec succes ou IN_PROGRESS= l'application est toujours en cours)
 */
running_e DEMO_dac_statemachine (bool_e ask_for_finish, char touch_pressed)
{
	typedef enum
	{
		INIT = 0,
		DISPLAY,
		ADJUST_DAC,
		CLOSE
	}state_e;
	running_e ret = IN_PROGRESS;
	static state_e state = INIT;
	switch(state)
	{
		case INIT:
			DAC_init_and_configure(); //Initialisation du dac
			state = DISPLAY;
			break;
		case DISPLAY:{
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_DrawRect(80-1,40-1,200+2,50+2,LCD_COLOR_RED);
			LCD_SetFont(&Font8x8);
			LCD_DisplayStringLine(LINE(18),COLUMN(0),(uint8_t *)"      Press cursor to change output",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_NO_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(21),COLUMN(0),(uint8_t *)"           DAC2  /  (PA5)",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_SetFont(&Font12x12);
			printf("Touch -> %% DAC\n");
			uint8_t i;
			for(i=0;i<=9;i++)
				printf("   %d  -> %3d%%\n", i, i*10);
			printf("   *  -> 100%%\n");
			printf("   +  -> n+1%%\n");
			printf("   -  -> n-1%%\n");
			LCD_SetFont(&Font12x12);
			state = ADJUST_DAC;
			break;}
		case ADJUST_DAC:{	//L'accolade ici permet de definir des variables locales juste en dessous, c'est un peu moche, mais assez pratique.
			uint16_t x,y;
			static uint8_t value;
			bool_e value_updated = FALSE;
			char text[10];
			if(!t)
			{
				t = 50;
				if(TS_Get_Touch(&x,&y)) //On lit des coordonnees entrees par l'utilisateur
				{
					if(x>80 && x<280)	//Si elles sont dans la bonne "bande" de l'ecran
					{
						value = (x-80)/2;		//On convertit cette entree en un pourcentage de la tension maximum de sortie
						value_updated = TRUE;
					}
				}
			}

			switch(touch_pressed)
			{
				case '+':	if(value<100)	value++; 	value_updated = TRUE;	break;
				case '-':	if(value>0)		value--;	value_updated = TRUE;	break;
				case '0':	value=0; 			value_updated = TRUE;	break;
				case '1':	value=10; 			value_updated = TRUE;	break;
				case '2':	value=20; 			value_updated = TRUE;	break;
				case '3':	value=30; 			value_updated = TRUE;	break;
				case '4':	value=40; 			value_updated = TRUE;	break;
				case '5':	value=50; 			value_updated = TRUE;	break;
				case '6':	value=60; 			value_updated = TRUE;	break;
				case '7':	value=70; 			value_updated = TRUE;	break;
				case '8':	value=80; 			value_updated = TRUE;	break;
				case '9':	value=90; 			value_updated = TRUE;	break;
				case '*':	value=100; 			value_updated = TRUE;	break;
				default:												break;
			}

			if(value_updated)
			{
				DAC_set_value(value);	//On applique la valeur correspondante au dac
				sprintf(text,"%3d%%",value);
				LCD_DisplayStringLine(LINE(5),COLUMN(0),(uint8_t *)text,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				if(value > 0)
					LCD_DrawFullRect(80,40,(value*2),50,LCD_COLOR_RED,LCD_COLOR_GREEN);
				if(value < 100)
					LCD_DrawFullRect(80+(value*2),40,((100-value)*2),50,LCD_COLOR_RED,LCD_COLOR_WHITE);
			}
			if(ask_for_finish)
				state = CLOSE;
			break;}
		case CLOSE:
			if(__HAL_DAC_DISABLE(&DAC_HandleStructure, DAC_CHANNEL_2) != HAL_OK)	//On desacitve le dac
			{
				// Erreur a gerer
			}

			state = INIT;
			ret = END_OK;
			break;
		default:
			break;
	}

	return ret;
}

/** @brief 	fonction appelee par la routine d'interruption du timer toutes les ms
 * @func	void DEMO_adc_process_1ms(void)
 */
void DEMO_dac_process_1ms(void)
{
	if(t)
		t--;
}

/** @brief	fonction qui convertit un pourcentage recu en entree en une tension en sortie
 * @func	void DAC_set_value(uint8_t value)
 * @param	value le pourcentage de la tension de sortie a appliquer
 */
void DAC_set_value(uint8_t value)
{
	int16_t out;
	out = MIN(100,value);	//Ecretage...
	out = (((int32_t)(out))*4096)/100; //on convertit le pourcentage en sa valeur en volt
	if(HAL_DAC_SetValue(&DAC_HandleStructure, DAC_CHANNEL_2, DAC_ALIGN_12B_R, out) != HAL_OK) //On applique cette valeur en sortie
	{
		// Erreur a gerer
	}
}


/**
 * @brief	Cette fonction initialise la broche PA5 comme analogique et active le DAC.
 * @func void DAC_init_and_configure(void)
 */
void DAC_init_and_configure(void)
{
    DAC_ChannelConfTypeDef DAC_ChannelConfStructure;
    //Activation de l'horloge du DAC
	__HAL_RCC_DAC_CLK_ENABLE();
	
    //Activation de l'horloge du GPIO utilise soit le A
	__HAL_RCC_GPIOA_CLK_ENABLE();

	DAC_HandleStructure.Instance = DAC;
	if(HAL_DAC_Init(&DAC_HandleStructure) != HAL_OK)
	{
		// Erreur a gerer
	}
    //Appel a la fonction d'initialisation des registres du peripherique GPIOA en broche 5
	BSP_GPIO_PinCfg(GPIOA, GPIO_PIN_5, GPIO_MODE_ANALOG, GPIO_NOPULL, GPIO_SPEED_MEDIUM, 0);

	
	DAC_ChannelConfStructure.DAC_Trigger = DAC_TRIGGER_NONE;// Specifie le trigger externe pour le DAC.
	DAC_ChannelConfStructure.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;// Indique si le tampon de sortie du canal du DAC  est activee ou desactivee .
	HAL_DAC_ConfigChannel(&DAC_HandleStructure, &DAC_ChannelConfStructure, DAC_CHANNEL_2);
	
    //Le DAC 2 est en ENABLE
	if(__HAL_DAC_ENABLE(&DAC_HandleStructure, DAC_CHANNEL_2) != HAL_OK)
	{
		// Erreur a gerer
	}
	if(HAL_DAC_Start(&DAC_HandleStructure, DAC_CHANNEL_2) != HAL_OK) //On commence la conversion
	{
		// Erreur a gerer
	}
}
