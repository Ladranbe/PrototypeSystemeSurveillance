 /**
  *
  * \file main.c
  */

/* Includes ------------------------------------------------------------------*/

#include "stm32f4xx_hal.h"
#include "macro_types.h"
#include "main.h"
#include "demo.h"
#include "demo_with_lcd.h"
#include "config.h"
#include "stm32f4_uart.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4_sd.h"
#include "demo_camera.h"
#include "demo_accelerometer.h"
#include "stm32f4_accelerometer.h"

#define SIZE_DMA_TAB 		(160*120*2)


extern UART_HandleTypeDef UART_HandleStructure[6];
uint8_t dma_buffer[SIZE_DMA_TAB];



#if !DEMO_WITH_LCD
	/**
	  * @brief  Main program.
	  * @func int main(void)
	  * @param  None
	  * @retval None
	  */
	int main(void)
	{
		DEMO_init();
		while (1)
		{
			DEMO_process_main();
		}
	}
/#else
#endif

	/**
	  * @brief  Main program.
	  * @func int main(void)
	  * @param  None
	  * @retval None
	  */

	int16_t xyz[3];
	int flag_accelero;

	extern uint32_t t;
	extern uint32_t t_ms; //Dur�e en ms
	static volatile bool_e flag_new_value; //pour informer la t�che de fond
	typedef enum {
		INIT,
		WAIT_BUTTON,
		WAIT_10MS,
		BLINK_500MS,
		BLINK_100MS,
		RUN_TRIGGER
	}etat_e;
	static etat_e etat=INIT;
	//enumeration des etats pour la machine � �tat capteur de pr�sence
	typedef enum {
			INITIALISATION,
			WAIT_PRESENCE,
			ACTIVATE_CAMERA,
			CAPTURE
		}et_e;
		static et_e et=INITIALISATION;

		extern int capteur_IT;
		extern int bouton_IT;

		bool_e press;

	/*void machineAetats(void){
		switch(etat)
		{
		case INIT:
			etat=WAIT_BUTTON;
			TIMER2_run_1ms();
			t=0;
			break;
		case WAIT_BUTTON:
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)){
				t=3000;
				etat=BLINK_500MS;
			}
			break;
		case BLINK_500MS:
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,(t/500)%2);
			if(t==0){
				t=2000;
				etat=BLINK_100MS;
			}
			break;
		case BLINK_100MS:
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,(t/100)%2);
			if(t==0){
				etat=RUN_TRIGGER;
			}
			break;
		case RUN_TRIGGER:
			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,1);
			t=10;
			etat=WAIT_10MS;
			break;
		case WAIT_10MS:
			if(t==0){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,0);
				etat=WAIT_BUTTON;
			}
			break;
		default:
			break;
		}
	}*/
	/*void EXTI0_IRQHandler(void)
					{
					if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) //Normalement forc�ment vrai
					{
						__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0); //On acquitte l'interruption.
						if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) //Front montant bouton bleu
						{
							if(etat==WAIT_BUTTON){
								t=3000;
								TIMER2_run_1ms();
								etat=BLINK_500MS;
								//flag_new_value=TRUE;
								/*t_ms=0;
								TIMER2_run_1ms(); //On lance le timer
							}
						}
						/*else //Front descendant : si on rel�che le bouton
						{
							TIMER2_stop(); //On arr�te le timer
							flag_new_value = TRUE; //On l�ve le flag
						}
					}
			}*/
	void machineAetats_capteur(void){
		switch(et)
		{
		case INITIALISATION:
			flag_accelero=0;
			BSP_ACCELERO_Init();
			et=WAIT_PRESENCE;
			xyz[0]=0;
			xyz[1]=0;
			xyz[2]=0;
			capteur_IT=0;
			DEMO_with_lcd_init();
			break;
		case WAIT_PRESENCE:
			break;
		case ACTIVATE_CAMERA:
			display_Camera_On_LCD();
			break;
		case CAPTURE:
			break;
		default:
			break;
		}
	}

	void EXTI0_IRQHandler(void){
		if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) //Normalement forc�ment vrai
		{
			__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0); 		//On acquitte l'interruption.
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)) 	//Front montant bouton bleu
			{
				if(BSP_SD_Init()==MSD_OK && BSP_SD_IsDetected()==SD_NOT_PRESENT){
					Capture_Image_To_BMP("0:img.bmp");
					printf("Capture enregistree\n");
				}
			}
		}
	}

	void EXTI1_IRQHandler(void){						//Interruption sur front montant du d�tecteur de pr�sence
		int i;
		if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET) //Normalement forc�ment vrai
		{
			__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1); 		//On acquitte l'interruption.
			if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_1)||flag_accelero==1)		//Front montant capteur mouvement
			{
				flag_accelero=0;
				UART_init(2,UART_RECEIVE_ON_MAIN);
				SYS_set_std_usart(USART2,USART2,USART2);
				printf("Presence intrus\n");			//Message diffus� sur PC par communication sur l'UART2
				printf("La camera est affichee sur le LCD\n");
				printf("Appuyer sur le bouton noir pour renitialiser le syst�me\n");
				et=ACTIVATE_CAMERA;
				NVIC_DisableIRQ(EXTI1_IRQn);			//D�sactivation de cette interruption afin de ne pas y rerentrer � chaque front montant du d�tecteur
			}
		}
	}

	int main(void)
	{
		//DEMO_with_lcd_init();					//Initialisation LCD
		//HAL_Init();
		//SYS_init();
		//GPIO_Configure();
		while (1)
		{
			machineAetats_capteur();
			if(et==WAIT_PRESENCE){
				DEMO_ACCELERO_SetPorts(PORTS_FOR_ACCELERO);		//on configure les ports pour pouvoir lire les donnees dans l'accelerometre...
				BSP_ACCELERO_GetXYZ(xyz);						//...on les lit...
				DEMO_ACCELERO_SetPorts(PORTS_FOR_LCD);
				if(xyz[2]<600 || xyz[2]>1000){
					flag_accelero=1;
					//et=ACTIVATE_CAMERA;
					//BSP_ACCELERO_DeInit();
					//printf("Une tierce personne manipule le boitier de l'alarme\n");
				}
			}
			//machineAetats();
			/*if(flag_new_value)
			{
				flag_new_value = FALSE;
				printf("Duree de l'appui en ms : %lu\n",t_ms);
			}*/

			/*BSP_ACCELERO_GetXYZ(xyz);
			if(xyz[0]>-55 && xyz[0]<55 && xyz[1]>-30 && xyz[1]<15 && xyz[2]>1000 && xyz[2]<1070){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,0);
			}else if(xyz[0]>-55 && xyz[0]<30 && xyz[1]>700 && xyz[1]<1000 && xyz[2]>400 && xyz[2]<700){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,0);
			}else if(xyz[0]>-150 && xyz[0]<50 && xyz[1]>-1100 && xyz[1]<-500 && xyz[2]>200 && xyz[2]<700){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,0);
			}else if(xyz[0]>500 && xyz[0]<1000 && xyz[1]>-100 && xyz[1]<150 && xyz[2]>300 && xyz[2]<700){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,0);
			}else if(xyz[0]>-1100 && xyz[0]<-600 && xyz[1]>-150 && xyz[1]<150 && xyz[2]>200 && xyz[2]<700){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,1);
			}
			else{
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,1);
				}*/
			//DEMO_with_lcd_process_main();
			/*if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)){
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,1);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,1);
			}else{
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,0);
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,0);
			}*/
		}
	}










