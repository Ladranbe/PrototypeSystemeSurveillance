/**
 * demo_pwm.c
 *
 *  Created on: 3 mars 2015
 *      Author: Nirgal
 */
#include "demo_PWM.h"
#include "stm32f4xx_hal.h"
#include "demo_touchscreen.h"
#include "lcd_display_form_and_text.h"
#include "stm32f4_gpio.h"


static volatile uint8_t t = 0;


void PWM_TIM4_CH2_set_duty(uint16_t duty);
void PWM_TIM4_CH2_init(void);

/**
 * @brief 	Cette fonction est une machine a etat qui regit le comportement de la demo du pwm.
 * @func 	running_e DEMO_pwm_statemachine (bool_e ask_for_finish)
 * @param 	exit_asked: demande a l'application de quitter ou non 
 * @return	cette fonction retourne un element de l'enumeration running_e (END_OK= l'application est quittee avec succes ou IN_PROGRESS= l'application est toujours en cours)
 */
running_e DEMO_pwm_statemachine (bool_e ask_for_finish, char touch_pressed)
{
	typedef enum
	{
		INIT = 0,
		DISPLAY,
		ADJUST_PWM,
		CLOSE
	}state_e;
	running_e ret = IN_PROGRESS;
	static state_e state = INIT;
	switch(state)
	{
		case INIT:
			PWM_TIM4_CH2_init();	//On initialise le pwm avec le timer4 sur le cannal2
			state = DISPLAY;
			break;
		case DISPLAY:{
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_DrawRect(80-1,40-1,200+2,50+2,LCD_COLOR_RED);
			LCD_SetFont(&Font8x8);
			LCD_DisplayStringLine(LINE(18),COLUMN(0),(uint8_t *)"      Press cursor to change duty",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_NO_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(21),COLUMN(0),(uint8_t *)"           TIM4 / Channel 2",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(24),COLUMN(0),(uint8_t *)"  PD13 : orange LED and LCD brightness",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			printf("Touch -> %% PWM\n");
			uint8_t i;
			for(i=0;i<=9;i++)
				printf("   %d  -> %3d%%\n", i, i*10);
			printf("   *  -> 100%%\n");
			printf("   +  -> n+1%%\n");
			printf("   -  -> n-1%%\n");
			LCD_SetFont(&Font12x12);
			state = ADJUST_PWM;
			break;}
		case ADJUST_PWM:{
			uint16_t x,y;
			static uint8_t duty = 100;
			bool_e duty_updated = FALSE;
			char text[10];
			if(!t)
			{
				t = 50;
				if(TS_Get_Touch(&x,&y)) //On lit une entree de l'utilisateur
				{
					if(x>80 && x<280)	//On vérifie que l'entrée soit dans la "bande" de l'ecran qui nous interesse
					{
						duty = (x-80)/2;	//On convertit cette entree en pourcentage
						duty_updated = TRUE;
					}
				}
			}

			switch(touch_pressed)
			{
				case '+':	if(duty<100)	duty++;		duty_updated = TRUE;	break;
				case '-':	if(duty>0)		duty--;		duty_updated = TRUE;	break;
				case '0':	duty=0; 			duty_updated = TRUE;	break;
				case '1':	duty=10; 			duty_updated = TRUE;	break;
				case '2':	duty=20; 			duty_updated = TRUE;	break;
				case '3':	duty=30; 			duty_updated = TRUE;	break;
				case '4':	duty=40; 			duty_updated = TRUE;	break;
				case '5':	duty=50; 			duty_updated = TRUE;	break;
				case '6':	duty=60; 			duty_updated = TRUE;	break;
				case '7':	duty=70; 			duty_updated = TRUE;	break;
				case '8':	duty=80; 			duty_updated = TRUE;	break;
				case '9':	duty=90; 			duty_updated = TRUE;	break;
				case '*':	duty=100; 			duty_updated = TRUE;	break;
				default:												break;
			}

			if(duty_updated)
			{
				PWM_TIM4_CH2_set_duty((uint16_t)duty); // On applique ce pourcentage au PWM
				sprintf(text,"%3d%%",duty);
				LCD_DisplayStringLine(LINE(5),COLUMN(0),(uint8_t *)text,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				if(duty > 0)
					LCD_DrawFullRect(80,40,(duty*2),50,LCD_COLOR_RED,LCD_COLOR_GREEN);
				if(duty < 100)
					LCD_DrawFullRect(80+(duty*2),40,((100-duty)*2),50,LCD_COLOR_RED,LCD_COLOR_WHITE);
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
void DEMO_pwm_process_1ms(void)
{
	if(t)
		t--;
}



#define TIM1_8_9_10_11_CLK    			168000000	//Fréquence des évènements d'horloge pour les timers 1, 8, 9, 10, 11
#define TIM2_3_4_5_6_7_12_13_14_CLK    	84000000	//Fréquence des évènements d'horloge pour les timers 2, 3, 4, 5, 6, 7, 12, 13 et 14

//L'horloge du timer 4 est a 84MHz
//Si l'on veut une PWM a 20kHz (inaudible) et 100 pas de rapports cycliques possibles, il faut prediviser par 42 :
//168MHz/84 = 2MHz -> 500ns par pas... * 100 pas = 20kHz de frequence PWM
#define PWM_FREQ_TIM4	20000 	//Fréquence du signal PWM, en Hz
#define	PWM_PERIOD_TIM4	100		//Nombre jusqu'auquel le timer va compter durant une période PWM

#define	PWM_PRESC_TIM4	((TIM2_3_4_5_6_7_12_13_14_CLK/PWM_FREQ_TIM4)/PWM_PERIOD_TIM4)	//Prédiviseur : nombre d'évènements qui provoquent +1 sur le décompte du timer

/*
 * Explications :
 * - Le timer compte des évènements...
 * - Nous disposons d'une horloge à 84MHz...
 * - Tout les X évènements (les X périodes d'horloge), le timer compte +1.... Ce X est le prédiviseur. (PWM_PRESC)
 * - Le timer va compter jusqu'à "PWM_PERIOD"... Puis revenir à 0. Et ainsi de suite.
 * - La durée qu'il met à compter jusqu'à cette PWM_PERIOD nous donne la période du signal PWM... L'inverse est la fréquence !
 * 				Période du signal PWM 	= (PWM_PERIOD	*	PWM_PRESC)/84MHz
 * 	Exemple :				50µs	 	= (100			*	42)/84MHz
 * 	 			Fréquence du signal PWM = 84MHz/(PWM_PERIOD	*	PWM_PRESC)
 * 	Exemple :				50µs	 	= 84MHz/(100		*	42)
 */


static TIM_HandleTypeDef TimHandle_4;	//Structure contenant les infos concernant l'état du timer 4

/** @brief Fonction qui configure et initialise le timer 4
 * @func void PWM_TIM4_CH2_init(void)
 */
void PWM_TIM4_CH2_init(void)
{
	TimHandle_4.Instance = TIM4;
	TIM_OC_InitTypeDef TIM_OCInitStructure;

	/* Configuration GPIO et remappings */
	//On configure la broche D13 pour le mode AF (Alternate Function), associée au périhérique TIM4.
	//Cette broche ne sera donc plus contrôlable comme une entrée sortie à usage général (GPIO)... On l'a confiée au timer TIM4.
	BSP_GPIO_PinCfg(GPIOD, GPIO_PIN_13, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FAST, GPIO_AF2_TIM4);

	__HAL_RCC_TIM4_CLK_ENABLE(); //On autorise la clock du timer 4
	
	
	TimHandle_4.Init.Period = PWM_PERIOD_TIM4 - 1;//Le timer compte de 0 a period inclus
	TimHandle_4.Init.Prescaler = PWM_PRESC_TIM4 - 1;
	TimHandle_4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // <=> 0
	TimHandle_4.Init.CounterMode = TIM_COUNTERMODE_UP;
	
	if(HAL_TIM_Base_Init(&TimHandle_4) != HAL_OK) //on initialise le timer 4
	{
		///gestion de l'erreur
	}
	
	TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
	TIM_OCInitStructure.Pulse = 0;
	TIM_OCInitStructure.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_OCInitStructure.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	TIM_OCInitStructure.OCFastMode = TIM_OCFAST_DISABLE; //disable the fast state
	
	//CHANNEL 2
	HAL_TIM_OC_Init(&TimHandle_4);
	HAL_TIM_OC_ConfigChannel(&TimHandle_4, &TIM_OCInitStructure, TIM_CHANNEL_2); //on configure le canal (avant on autorisait le prechargement de la config)

	__HAL_TIM_ENABLE(&TimHandle_4);
	HAL_TIM_PWM_Start(&TimHandle_4, TIM_CHANNEL_2);

	//Pour modifier le rapport cyclique :
	PWM_TIM4_CH2_set_duty(3*PWM_PERIOD_TIM4 / 4);	//75% par defaut
}


/** @brief	fonction qui convertit un pourcentage recu en entree en un rapport cyclique correspondant
 * @func	void PWM_TIM4_CH2_set_duty(uint8_t duty)
 * @param	duty le rapport cyclique a appliquer
 */
void PWM_TIM4_CH2_set_duty(uint16_t duty)
{
	duty = MIN(PWM_PERIOD_TIM4,duty);	//Ecretage... Le rapport cyclique est exprimé dans la même unité que la PWM_PERIOD, il ne peut donc pas être plus grand !
	__HAL_TIM_SET_COMPARE(&TimHandle_4, TIM_CHANNEL_2, (uint32_t)duty);
}

