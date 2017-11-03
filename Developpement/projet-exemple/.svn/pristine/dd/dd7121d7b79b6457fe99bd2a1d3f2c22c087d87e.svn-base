/**
 * demo_motor.c
 *
 *  Created on: 27 octobre 2015
 *      Author: S. Poiraud
 */
#include "demo_PWM.h"
#include "stm32f4xx_hal.h"
#include "stm32f4_gpio.h"

typedef enum
{
	MOTOR1,
	MOTOR2,
	MOTOR3,
	MOTOR4,
	MOTOR_NB
}motor_e;


void MOTOR_set_duty(int16_t duty, motor_e motor);
void MOTOR_init(void);

/**
 * @brief 	Cette fonction est une machine a etat qui regit le comportement de la demo du pwm.
 * @func 	running_e DEMO_pwm_statemachine (bool_e ask_for_finish)
 * @param 	exit_asked: demande a l'application de quitter ou non 
 * @return	cette fonction retourne un element de l'enumeration running_e (END_OK= l'application est quittee avec succes ou IN_PROGRESS= l'application est toujours en cours)
 */
running_e DEMO_MOTOR_statemachine (bool_e ask_for_finish, char touch_pressed)
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
			MOTOR_init();	//On initialise le pwm avec le timer4 sur le cannal2
			state = DISPLAY;
			break;
		case DISPLAY:{
			printf("TIM1 / Channels 1, 2, 3, 4, 1N, 2N, 3N, 4N\n");
			printf("Touch -> %% PWM\n");
			uint8_t i;
			for(i=0;i<=9;i++)
				printf("   %d  -> %3d%%\n", i, i*10);
			printf("   *  -> 100%%\n");
			printf("   +  -> n+1%%\n");
			printf("   -  -> n-1%%\n");
			for(i=0;i<4;i++)
				printf("   %c  -> Channel %d\n", 'A'+i, i+1);
			state = ADJUST_PWM;
			break;}
		case ADJUST_PWM:{
			static int8_t duty = 50;
			static motor_e motor = TIM_CHANNEL_1;
			bool_e duty_updated = FALSE;
			bool_e motor_updated = FALSE;

			switch(touch_pressed)
			{
				case '+':	if(duty<100)		duty++;		duty_updated = TRUE;	break;
				case '-':	if(duty>-100)		duty--;		duty_updated = TRUE;	break;
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
				case 'a':	case 'A':	motor = MOTOR1;	motor_updated = TRUE;	break;
				case 'b':	case 'B':	motor = MOTOR2;	motor_updated = TRUE;	break;
				case 'c':	case 'C':	motor = MOTOR3;	motor_updated = TRUE;	break;
				case 'd':	case 'D':	motor = MOTOR4;	motor_updated = TRUE;	break;
				default:												break;
			}

			if(motor_updated)
			{
				printf("Motor %d\n",motor+1);
			}
			if(duty_updated)
			{
				MOTOR_set_duty((int16_t)duty, motor); // On applique ce pourcentage au PWM
				printf("Motor %d%c -> %3d%%\n", motor+1, (duty<0)?'N':' ', duty);
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




#define TIM1_8_9_10_11_CLK    			168000000	//Fréquence des évènements d'horloge pour les timers 1, 8, 9, 10, 11
#define TIM2_3_4_5_6_7_12_13_14_CLK    	84000000	//Fréquence des évènements d'horloge pour les timers 2, 3, 4, 5, 6, 7, 12, 13 et 14

//L'horloge du timer 1 est a 168MHz
//Si l'on veut une PWM a 20kHz (inaudible) et 100 pas de rapports cycliques possibles, il faut prediviser par 42 :
//168MHz/84 = 2MHz -> 500ns par pas... * 100 pas = 20kHz de frequence PWM
#define PWM_FREQ	20000 	//Fréquence du signal PWM, en Hz
#define	PWM_PERIOD	100		//Nombre jusqu'auquel le timer va compter durant une période PWM

#define	PWM_PRESC_TIM1	((TIM1_8_9_10_11_CLK/PWM_FREQ)/PWM_PERIOD)			//Prédiviseur : nombre d'évènements qui provoquent +1 sur le décompte du timer
#define	PWM_PRESC_TIM3	((TIM2_3_4_5_6_7_12_13_14_CLK/PWM_FREQ)/PWM_PERIOD)	//Prédiviseur : nombre d'évènements qui provoquent +1 sur le décompte du timer

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


static TIM_HandleTypeDef TimHandle_1;	//Structure contenant les infos concernant l'état du timer 1
static TIM_HandleTypeDef TimHandle_3;	//Structure contenant les infos concernant l'état du timer 3

/** @brief Fonction qui configure et initialise les timers 1 et 3...
 * @func void PWM_TIM4_CH2_init(void)
 */
void MOTOR_init(void)
{
	TIM_OC_InitTypeDef TIM_OCInitStructure;

	__HAL_RCC_TIM1_CLK_ENABLE(); //On autorise la clock du timer 1
	__HAL_RCC_TIM3_CLK_ENABLE(); //On autorise la clock du timer 3
	
	TimHandle_1.Instance = TIM1;
	TimHandle_1.Init.Period = PWM_PERIOD - 1;//Le timer compte de 0 a period inclus
	TimHandle_1.Init.Prescaler = PWM_PRESC_TIM1 - 1;
	TimHandle_1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // <=> 0
	TimHandle_1.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TimHandle_1); //on initialise le timer 1
	
	TimHandle_3.Instance = TIM3;
	TimHandle_3.Init.Period = PWM_PERIOD - 1;//Le timer compte de 0 a period inclus
	TimHandle_3.Init.Prescaler = PWM_PRESC_TIM3 - 1;
	TimHandle_3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // <=> 0
	TimHandle_3.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TimHandle_3); //on initialise le timer 3
	
	TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
	TIM_OCInitStructure.Pulse = 25;  	//Par défaut : 25% de rapport cyclique.
	TIM_OCInitStructure.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_OCInitStructure.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	TIM_OCInitStructure.OCFastMode = TIM_OCFAST_DISABLE; 	//disable the fast state
	TIM_OCInitStructure.OCIdleState = TIM_OCIDLESTATE_SET;	//Active la sortie
	TIM_OCInitStructure.OCNIdleState = TIM_OCNIDLESTATE_SET;//Active la sortie 'N'
	
	HAL_TIM_PWM_ConfigChannel(&TimHandle_1,&TIM_OCInitStructure,TIM_CHANNEL_1);
	HAL_TIM_PWM_ConfigChannel(&TimHandle_1,&TIM_OCInitStructure,TIM_CHANNEL_2);
	HAL_TIM_PWM_ConfigChannel(&TimHandle_1,&TIM_OCInitStructure,TIM_CHANNEL_3);
	HAL_TIM_PWM_ConfigChannel(&TimHandle_3,&TIM_OCInitStructure,TIM_CHANNEL_1);
	HAL_TIM_PWM_ConfigChannel(&TimHandle_3,&TIM_OCInitStructure,TIM_CHANNEL_2);

	__HAL_TIM_ENABLE(&TimHandle_1);
	__HAL_TIM_ENABLE(&TimHandle_3);

	HAL_TIM_PWM_Start(&TimHandle_1, TIM_CHANNEL_1);		//1.CH1
	HAL_TIM_PWM_Start(&TimHandle_1, TIM_CHANNEL_2);		//1.CH2
	HAL_TIM_PWM_Start(&TimHandle_1, TIM_CHANNEL_3);		//1.CH2
	HAL_TIMEx_PWMN_Start(&TimHandle_1, TIM_CHANNEL_1);	//1.CH1N
	HAL_TIMEx_PWMN_Start(&TimHandle_1, TIM_CHANNEL_2);	//1.CH2N
	HAL_TIMEx_PWMN_Start(&TimHandle_1, TIM_CHANNEL_3);	//1.CH3N
	HAL_TIM_PWM_Start(&TimHandle_3, TIM_CHANNEL_1);		//3.CH1
	HAL_TIM_PWM_Start(&TimHandle_3, TIM_CHANNEL_2);		//3.CH2
}


/** @brief	fonction qui convertit un pourcentage recu en entree en un rapport cyclique correspondant
 * @func	void PWM_set_duty(uint16_t duty, channel_e channel)
 * @param	duty le rapport cyclique a appliquer
 * @param	channel TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3 ou TIM_CHANNEL_4
 */
void MOTOR_set_duty(int16_t duty, motor_e motor)
{
	bool_e reverse;
	GPIO_TypeDef * port_to_configure_in_gpio;
	GPIO_TypeDef * port_to_configure_in_pwm;
	uint16_t pin_to_configure_in_gpio, pin_to_configure_in_pwm;
	uint32_t channel;
	uint8_t mapping;
	TIM_HandleTypeDef * handler;

	reverse = (duty < 0)?TRUE:FALSE;

	//Ecretage... Le rapport cyclique est exprimé dans la même unité que la PWM_PERIOD, il ne peut donc pas être plus grand !
	if(duty > 100)
		duty = 100;
	else if(duty < -100)
		duty = -100;

	if(reverse)
		duty += 100;		// - 75 devient  25%

	/* Configuration GPIO et remappings
	 *
	 * Pour chaque moteur, on dispose de deux signaux PWM.
	 * Selon le sens demandé, on configure l'une des deux broches en AF, et l'autre en GPIO :
			-> la broche configurée pour le mode AF (Alternate Function), ne sera donc plus contrôlable comme une entrée sortie à usage général (GPIO)...
					On l'a confiée au timer et on peut y voir la PWM demandée.
			-> l'autre broche est configurée en mode OUTPUT, pour la maintenir à 0.
	*/

	switch(motor)
	{
		case MOTOR1:
			port_to_configure_in_gpio 	= GPIOE;
			port_to_configure_in_pwm 	= GPIOE;
			pin_to_configure_in_gpio 	= (reverse)?GPIO_PIN_9:GPIO_PIN_8;
			pin_to_configure_in_pwm 	= (reverse)?GPIO_PIN_8:GPIO_PIN_9;
			channel						= TIM_CHANNEL_1;
			handler						= &TimHandle_1;
			mapping 					= GPIO_AF1_TIM1;
			break;
		case MOTOR2:
			port_to_configure_in_gpio 	= GPIOE;
			port_to_configure_in_pwm 	= GPIOE;
			pin_to_configure_in_gpio 	= (reverse)?GPIO_PIN_11:GPIO_PIN_10;
			pin_to_configure_in_pwm 	= (reverse)?GPIO_PIN_10:GPIO_PIN_11;
			channel						= TIM_CHANNEL_2;
			handler						= &TimHandle_1;
			mapping 					= GPIO_AF1_TIM1;
			break;
		case MOTOR3:
			port_to_configure_in_gpio 	= GPIOE;
			port_to_configure_in_pwm 	= GPIOE;
			pin_to_configure_in_gpio 	= (reverse)?GPIO_PIN_13:GPIO_PIN_12;
			pin_to_configure_in_pwm 	= (reverse)?GPIO_PIN_12:GPIO_PIN_13;
			channel						= TIM_CHANNEL_3;
			handler						= &TimHandle_1;
			mapping 					= GPIO_AF1_TIM1;
			break;
		case MOTOR4:
			port_to_configure_in_gpio 	= GPIOB;
			port_to_configure_in_pwm 	= GPIOB;
			pin_to_configure_in_gpio 	= (reverse)?GPIO_PIN_4:GPIO_PIN_5;
			pin_to_configure_in_pwm 	= (reverse)?GPIO_PIN_5:GPIO_PIN_4;
			channel						= (reverse)?TIM_CHANNEL_2:TIM_CHANNEL_1;
			handler						= &TimHandle_3;
			mapping 					= GPIO_AF2_TIM3;
			break;
		default:
			return;
			break;
	}

	BSP_GPIO_PinCfg(port_to_configure_in_pwm, pin_to_configure_in_pwm,  GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FAST, mapping);
	BSP_GPIO_PinCfg(port_to_configure_in_gpio, pin_to_configure_in_gpio,  GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FAST, 0);
	__HAL_TIM_SET_COMPARE(handler, channel, (uint32_t)duty);
}

