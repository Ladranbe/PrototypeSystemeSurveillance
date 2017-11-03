/*
 * demo_usb_device.c
 *
 *  Created on: 3 mars 2015
 *      Author: Nirgal
 */

#include "demo_usb_device.h"
#include "demo_touchscreen.h"
#include "lcd_display_form_and_text.h"
#include "stm32f4xx_hal.h"
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_hid.h"
#include "stm32f4_gpio.h"



volatile static uint8_t t = 0;
USBD_HandleTypeDef  hUSBDDevice;
extern PCD_HandleTypeDef hpcd;

static uint32_t Demo_USBConfig(void);

running_e DEMO_usb_device_statemachine (bool_e ask_for_finish, char touch_pressed)
{
	typedef enum
	{
		INIT = 0,
		DISPLAY,
		NOT_IN_DEVICE_MODE,
		RUN,
		CLOSE
	}state_e;
	running_e ret = IN_PROGRESS;
	static state_e state = INIT;
	switch(state)
	{
		case INIT:


#ifdef USE_DEVICE_MODE
			state = DISPLAY;
			/*USBD_Init(&USB_OTG_dev,
				#ifdef USE_USB_OTG_HS
							USB_OTG_HS_CORE_ID,
				#else
							USB_OTG_FS_CORE_ID,
				#endif
							&USR_desc,
							&USBD_HID_cb,
							&USR_cb);*/
			Demo_USBConfig();
#else
			state = NOT_IN_DEVICE_MODE;
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_SetFont(&Font8x8);
			LCD_DisplayStringLine(LINE(18),COLUMN(0),(uint8_t *)"Software should be recompiled",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(21),COLUMN(0),(uint8_t *)" With USE_DEVICE_MODE",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(24),COLUMN(0),(uint8_t *)"  See middleware/usb_readme.txt",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_SetFont(&Font12x12);
#endif
			break;
		case DISPLAY:
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_SetFont(&Font8x8);
			LCD_DisplayStringLine(LINE(25),COLUMN(0),(uint8_t *)"  Plug USB micro AB to a computer",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(27),COLUMN(0),(uint8_t *)"  And Play with this touchscreen",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_NO_DISPLAY_ON_UART);
			LCD_SetFont(&Font12x12);
			printf("Touch -> action on mouse\n");
			printf("   z  -> up\n");
			printf("   s  -> down\n");
			printf("   q  -> left\n");
			printf("   d  -> right\n");
			printf("   c  -> left-clic\n");
			printf("   v  -> release left-clic\n");
			state = RUN;
			break;
		case NOT_IN_DEVICE_MODE:
			if(ask_for_finish)
				state = CLOSE;
			break;
		case RUN:{
				uint8_t Buffer_usb[5];
				uint16_t x,y;
				bool_e current_press;
				bool_e data_updated;
				static bool_e previous_press;
				static uint16_t press_duration;
				static int16_t previous_x;
				static int16_t previous_y;

				data_updated = FALSE;

				Buffer_usb[0] = 0x02;    // SOURIS !

				Buffer_usb[1] = 0;    // clics
				Buffer_usb[2] = 0;    // X de la souris
				Buffer_usb[3] = 0;    // Y de la souris
				Buffer_usb[4] = 0;    // roulette

				if(!t)
				{
					t = 10;

					current_press = TS_Get_Touch(&x,&y);
					if(!previous_press && current_press)		//On vient d'appuyer
					{	//Reset position precedente
						previous_x = x;
						previous_y = y;
						press_duration = 0;
					}
					else if(current_press)		//On maintient l'appui
					{
						if(press_duration < 10000)
							press_duration += 10;
						Buffer_usb[2] = x-previous_x;    // X de la souris
						Buffer_usb[3] = y-previous_y;    // Y de la souris
						data_updated = TRUE;
					}
					else if(previous_press)		//On vient de relâcher
					{
						if(press_duration < 100)
						{
							Buffer_usb[1] = 1;       // clics
							data_updated = TRUE;
						}
					}

					previous_x = (int16_t)x;
					previous_y = (int16_t)y;
					previous_press = current_press;
				}

				switch(touch_pressed)
				{
					case 'z':	Buffer_usb[3]=-10; 			data_updated = TRUE;	break;
					case 's':	Buffer_usb[3]=10; 			data_updated = TRUE;	break;
					case 'q':	Buffer_usb[2]=-10; 			data_updated = TRUE;	break;
					case 'd':	Buffer_usb[2]=10; 			data_updated = TRUE;	break;
					case 'c':	Buffer_usb[1]=1; 			data_updated = TRUE;	break;
					case 'v':	Buffer_usb[1]=0; 			data_updated = TRUE;	break;
					default:												break;
				}

					#ifdef USE_DEVICE_MODE
						if(data_updated)
						{
							// Envoi de donnees par USB s'il y a eu un changement d'etat de l'accelerometre ou du bouton
							if(Buffer_usb[1] || Buffer_usb[2] || Buffer_usb[3])
								USBD_HID_SendReport(&hUSBDDevice,Buffer_usb,5);
						}
					#endif

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


void DEMO_usb_device_process_1ms(void)
{
	if(t)
		t--;
}




#ifdef USE_DEVICE_MODE
	/**
	  * @brief  This function handles EXTI15_10_IRQ Handler.
	  * @param  None
	  * @retval None
	  */
	#ifdef USE_USB_OTG_FS
	void OTG_FS_WKUP_IRQHandler(void)
	{
	  //if(USB_OTG_dev.cfg.low_power)
	  //{
		/* Reset SLEEPDEEP and SLEEPONEXIT bits */
		SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));

		/* After wake-up from sleep mode, reconfigure the system clock */
		SystemInit();
		USBD_Start(&hUSBDDevice);
	  //}
		__HAL_USB_OTG_FS_WAKEUP_EXTI_CLEAR_FLAG();
	}
	#endif

	/**
	  * @brief  This function handles EXTI15_10_IRQ Handler.
	  * @param  None
	  * @retval None
	  */
	#ifdef USE_USB_OTG_HS
	void OTG_HS_WKUP_IRQHandler(void)
	{
	  if(USB_OTG_dev.cfg.low_power)
	  {
		/* Reset SLEEPDEEP and SLEEPONEXIT bits */
		SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));

		/* After wake-up from sleep mode, reconfigure the system clock */
		SystemInit();
		USBD_Start(&USB_OTG_dev);
	  }
	  EXTI_ClearITPendingBit(EXTI_Line20);
	}
	#endif

	/**
	  * @brief  This function handles OTG_HS Handler.
	  * @param  None
	  * @retval None
	  */
	#ifdef USE_USB_OTG_HS
	void OTG_HS_IRQHandler(void)
	#else
	void OTG_FS_IRQHandler(void)
	#endif
	{
		 HAL_PCD_IRQHandler(&hpcd);
	  //USBD_OTG_ISR_Handler (&USB_OTG_dev);
	}

	#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
	/**
	  * @brief  This function handles EP1_IN Handler.
	  * @param  None
	  * @retval None
	  */
	void OTG_HS_EP1_IN_IRQHandler(void)
	{
	  USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_dev);
	}

	/**
	  * @brief  This function handles EP1_OUT Handler.
	  * @param  None
	  * @retval None
	  */
	void OTG_HS_EP1_OUT_IRQHandler(void)
	{
	  USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_dev);
	}
	#endif
	
	static uint32_t Demo_USBConfig(void)
	{
		/* Init Device Library */
		USBD_Init(&hUSBDDevice, &HID_Desc, 0);

		LCD_Clear(LCD_COLOR_GREEN);
		/* Add Supported Class */
		USBD_RegisterClass(&hUSBDDevice, USBD_HID_CLASS);
		LCD_Clear(LCD_COLOR_RED);

		/* Start Device Process */
		if(USBD_Start(&hUSBDDevice) != USBD_OK)
		{
			LCD_Clear(LCD_COLOR_BLUE);
		}


	  
		return 0;
	}
#endif

