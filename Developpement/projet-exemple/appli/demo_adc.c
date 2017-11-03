/**
 * demo_adc.c
 *
 *  Created on: 3 mars 2015
 *      Author: Samuel Poiraud
 */
#include "demo_adc.h"
#include "stm32f4xx_hal.h"
#include "demo_touchscreen.h"
#include "lcd_display_form_and_text.h"
#include "stm32f4_gpio.h"

static volatile uint8_t t = 0;
void ADC1_init_for_channel_8_PB0(void);
ADC_HandleTypeDef ADC_HandleStructure;

/**
 * @brief 	Cette fonction est une machine a etat qui regit le comportement de la demo de l'adc.
 * @func 	running_e DEMO_adc_state_machine(bool_e ask_for_finish)
 * @param 	exit_asked: demande a l'application de quitter ou non 
 * @return	cette fonction retourne un element de l'enumeration running_e (END_OK= l'application est quittee avec succes ou IN_PROGRESS= l'application est toujours en cours)
 */
running_e DEMO_adc_statemachine (bool_e ask_for_finish)
{
	typedef enum
	{
		INIT = 0,
		DISPLAY,
		CLOSE
	}state_e;
	running_e ret = IN_PROGRESS;
	static state_e state = INIT;
	switch(state)
	{
		case INIT:
			ADC1_init_for_channel_8_PB0();		//initialisation de l'adc
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_DrawRect(80-1,40-1,200+2,50+2,LCD_COLOR_RED);
			LCD_SetFont(&Font8x8);
			LCD_DisplayStringLine(LINE(18),COLUMN(0),(uint8_t *)"  Analog to Digital Converter",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(21),COLUMN(0),(uint8_t *)"  Measure of channel 8 (PB0)",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_SetFont(&Font12x12);
			state = DISPLAY;
			break;
		case DISPLAY:{
			char text[20];
			uint16_t value;
			uint16_t volt;
			if(!t)
			{
				t = 10;
				while(__HAL_ADC_GET_FLAG(&ADC_HandleStructure, ADC_FLAG_EOC) == RESET); //tant que la conversion n'est pas finie on attend
				value = HAL_ADC_GetValue(&ADC_HandleStructure);							//On recupere la valeur mesuree par l'adc
				volt = (((int32_t)value)*3000)/4096;									//On la convertie en volts
				sprintf(text,"%4d : %1d.%03dV",value, volt/1000, volt%1000);
				LCD_DisplayStringLine(LINE(5),COLUMN(8),(uint8_t *)text,LCD_COLOR_RED,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			}
			if(ask_for_finish)
				state = CLOSE;
			break;}
		case CLOSE:
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
void DEMO_adc_process_1ms(void)
{
	if(t)
		t--;
}


/**
 * @brief	Cette fonction initialise la broche PB0 comme analogique, active l'ADC1 et la conversion en continu.
 * @func	void ADC1_init_for_channel_8_PB0(void)
 */
void ADC1_init_for_channel_8_PB0(void)
{
	//Activation de l'horloge du GPIO utilise
	__HAL_RCC_GPIOB_CLK_ENABLE();

	//Appel a la fonction d'initialisation des registres du peripherique GPIO
	BSP_GPIO_PinCfg(GPIOB, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_NOPULL, GPIO_SPEED_MEDIUM, 0);

	//Activation de l'horloge du peripherique ADC1
	__HAL_RCC_ADC1_CLK_ENABLE();
	
	//Initialisation de ADCx
	ADC_HandleStructure.Instance = ADC1;
	ADC_HandleStructure.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;		//Prediviseur des ADC (2, 4, 6 ou 8)
	ADC_HandleStructure.Init.Resolution = ADC_RESOLUTION_12B;					//resolution de l'adc (valeurs possibles : 6, 8, 10 ou 12 bits)
	ADC_HandleStructure.Init.DataAlign = ADC_DATAALIGN_RIGHT;					//Alignement a droite des donnees mesurees dans le resultat sur 16 bits.
	ADC_HandleStructure.Init.ScanConvMode = DISABLE;							//Mode de conversion : ENABLE si plusieurs canaux. DISABLE si un seul canal.
	ADC_HandleStructure.Init.EOCSelection = DISABLE; 
	ADC_HandleStructure.Init.ContinuousConvMode = ENABLE;						//Mode continu : ENABLE si mesures en continu. DISABLE si une seule mesure a effectuer.
	ADC_HandleStructure.Init.DMAContinuousRequests = DISABLE; 					//DMA desactive...
	ADC_HandleStructure.Init.NbrOfConversion = 1;								//Nombre de conversions a effectuer dans un cycle de mesure. (minimum 1, maximum 16).
	ADC_HandleStructure.Init.DiscontinuousConvMode = DISABLE; 
	ADC_HandleStructure.Init.NbrOfDiscConversion = 0; 
	ADC_HandleStructure.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_CC2;
	ADC_HandleStructure.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;//Reglage du declencheur de la mesure (en cas de declenchement externe) None si pas de declenchement externe.
	if(HAL_ADC_Init(&ADC_HandleStructure) != HAL_OK)							//Initialisation de l'ADCx avec la structure remplie
	{
		// Erreur a gerer
	}

	//Reglage de l'ADC souhaite sur le canal demande
	ADC_ChannelConfTypeDef sConfig;
	sConfig.Channel = ADC_CHANNEL_8;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	sConfig.Offset = 0;
	
	if(HAL_ADC_ConfigChannel(&ADC_HandleStructure, &sConfig) != HAL_OK)
	{
		// Erreur a gerer
	}
	
	
	//Desactivation de l'ADCx
	__HAL_ADC_DISABLE(&ADC_HandleStructure);
	//On lance les conversions (en continu)... C'est parti... (cette fonction comprend l'activation de l'ADCx)
	if(HAL_ADC_Start(&ADC_HandleStructure) != HAL_OK)
	{
		// Erreur a gerer
	}
	ADC_HandleStructure.Instance->CR2 |= (uint32_t)ADC_CR2_SWSTART;

	while(__HAL_ADC_GET_FLAG(&ADC_HandleStructure, ADC_FLAG_STRT) == RESET);
}

