/**
 * demo_usb_host.c
 *
 *  Created on: 2 mars 2015
 *      Author: Nirgal
 */

#include "macro_types.h"
#include "usbh_core.h"
#include "demo_usb_host.h"
#include "ff_gen_drv.h"
#include "usbh_msc.h"
#include "usbh_diskio.h"
#include "lcd_display_form_and_text.h"
#include "demo_sd.h"
#include "stm32f4_gpio.h"


 extern HCD_HandleTypeDef hhcd __unused;


 void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id) __unused;


FATFS usb_fatfs;           /* File system object for USB disk logical drive */
static FIL MyFile;                   /* File object */
char USBDISKPath[4];          /* USB Host logical drive path */
//USB_OTG_CORE_HANDLE          USB_OTG_Core;

#ifdef USE_HOST_MODE
USBH_HandleTypeDef                     hUSB_Host;

/**
 * @brief	Configuration des entrees/sorties des broches du microcontroleur.
 * @func	void GPIO_COnfigure(void)
 * @post	Activation des horloges des peripheriques GPIO, configuration en entree ou en sortie des broches choisies.
 */
void DEMO_USB_init(void)  ///a faire
{
	if(FATFS_LinkDriver(&USBH_Driver, USBDISKPath) == 0)
	{
	    USBH_Init(&hUSB_Host, USBH_UserProcess, 0);
		  LCD_Clear(LCD_COLOR_GREEN);
	    USBH_RegisterClass(&hUSB_Host, USBH_MSC_CLASS);
	    USBH_Start(&hUSB_Host);
	}
}



running_e DEMO_USB_state_machine (bool_e ask_for_finish)
{
	typedef enum
	{
		INIT = 0,
		WAIT_PHYSICAL_DRIVE,
		MOUNT,
		WRITE_TEST_FILE,
		READ_TEST_FILE,
		COPY_FILE,
		DELETE_FILE,
		END_WITH_ERROR,
		IDLE
	}state_e;

	static state_e state = INIT;
	FRESULT res;
	running_e ret = IN_PROGRESS;
	static uint8_t line = 0;

	switch(state)
	{
		case INIT:
			line = 1;
			LCD_Clear(LCD_COLOR_WHITE);

			DEMO_USB_init();
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_SetFont(&Font8x8);
			LCD_DisplayStringLine(LINE(2*(line++)),COLUMN(0),(uint8_t *)"  Plug USB Key in USB Host",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			state = WAIT_PHYSICAL_DRIVE;
			break;
		case WAIT_PHYSICAL_DRIVE:
			if(USBH_MSC_IsReady(&hUSB_Host))
			{
				LCD_DisplayStringLine(LINE(2*(line++)),COLUMN(0),(uint8_t *)"  USB key detected...",LCD_COLOR_BLUE,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				if(disk_initialize(PD_USB) == 0)
				{
					LCD_DisplayStringLine(LINE(2*(line++)),COLUMN(0),(uint8_t *)"  Mounting...",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
					state = MOUNT;
				}
				else
					state = END_WITH_ERROR;
			}
			if(ask_for_finish)
			{
				state = INIT;
				ret = END_OK;
			}
			break;
		case MOUNT:{
			char text[60];
			FATFS * fs;
			Uint32 p2;
			state = END_WITH_ERROR;	//On fait la supposition que ça ne va pas marcher.... si tout se passe bien, on ira faire la suite.
			res = f_mount(&usb_fatfs, (TCHAR const*)USBDISKPath, 1);
			if(res == FR_OK)
			{
				LCD_DisplayStringLine(LINE(2*(line++)),COLUMN(0),(uint8_t *)"  Mounted...",LCD_COLOR_GREEN,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);

				res = f_getfree((TCHAR const*)USBDISKPath, (DWORD*)&p2, &fs);
				if (res == FR_OK)
				{
					debug_printf("FAT type = %u (%s)\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
								"Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n"
								"FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\nData start (lba) = %lu\n\n",
								(WORD)fs->fs_type,
								(fs->fs_type==FS_FAT12) ? "FAT12" : (fs->fs_type==FS_FAT16) ? "FAT16" : "FAT32",
								(DWORD)fs->csize * 512, (WORD)fs->n_fats,
								fs->n_rootdir, fs->fsize, (DWORD)fs->n_fatent - 2,
								fs->fatbase, fs->dirbase, fs->database
					);
					sprintf(text,"  FAT%s - %lu clusters * %lu Bytes",
									(fs->fs_type==FS_FAT12) ? "12" : (fs->fs_type==FS_FAT16) ? "16" : "32",
									(DWORD)fs->n_fatent - 2,
									(DWORD)fs->csize * 512
							);
					LCD_DisplayStringLine(LINE(2*(line++)),COLUMN(0),(uint8_t*)text,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);

					state = WRITE_TEST_FILE;
				}
			}
			if(res != FR_OK)
			{
				LCD_DisplayStringLine(LINE(2*(line++)),COLUMN(0),(uint8_t *)"  fail...",LCD_COLOR_RED,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);

				verbose_fresult(res);
			}
			break;}

		//TODO : ajouter un menu avec des boutons pour acceder aux fonctions suivantes :
		case WRITE_TEST_FILE:
			state = READ_TEST_FILE;
			break;
		case READ_TEST_FILE:
			state = COPY_FILE;
			break;

		case COPY_FILE:
			state = DELETE_FILE;
			break;
		case DELETE_FILE:
			state = IDLE;
			break;
		case END_WITH_ERROR:
			LCD_DisplayStringLine(LINE(2*(line++)),COLUMN(0),(uint8_t *)"  end with error...",LCD_COLOR_RED,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			break;
		case IDLE:
			if(ask_for_finish)
			{
				state = INIT;
				ret = END_OK;
			}
			break;
		default:
			break;
	}

	/* Host Task handler */
	USBH_Process(&hUSB_Host);

	return ret;
}




void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{

}



	#ifdef USE_HOST_MODE

		/**
		  * @brief  OTG_FS_IRQHandler
		  *          This function handles USB-On-The-Go FS global interrupt request.
		  *          requests.
		  * @param  None
		  * @retval None
		  */

		#ifdef USE_USB_OTG_FS
		void OTG_FS_IRQHandler(void)
		#else
		void OTG_HS_IRQHandler(void)
		#endif
		{
			HAL_HCD_IRQHandler(&hhcd);
		}

		#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
		/**
		  * @brief  This function handles EP1_IN Handler.
		  * @param  None
		  * @retval None
		  */
		void OTG_HS_EP1_IN_IRQHandler(void)
		{
		  USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_Core);
		}

		/**
		  * @brief  This function handles EP1_OUT Handler.
		  * @param  None
		  * @retval None
		  */
		void OTG_HS_EP1_OUT_IRQHandler(void)
		{
		  USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_Core);
		}
		#endif
	#endif

#else


running_e DEMO_USB_state_machine (bool_e ask_for_finish)
{
	typedef enum
	{
		INIT = 0,
		IDLE,
		CLOSE
	}state_e;
	running_e ret = IN_PROGRESS;
	static state_e state = INIT;
	switch(state)
	{
		case INIT:
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_SetFont(&Font8x8);
			LCD_DisplayStringLine(LINE(18),COLUMN(0),(uint8_t *)"Software should be recompiled",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(21),COLUMN(0),(uint8_t *)" With USE_HOST_MODE",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			LCD_DisplayStringLine(LINE(24),COLUMN(0),(uint8_t *)" See  middleware/usb_readme.txt",LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			state = IDLE;
			break;
		case IDLE:
			if(ask_for_finish)
				state = CLOSE;
			break;
		case CLOSE:
			state = INIT;
			ret = END_OK;
			break;
		default:
			break;
	}

	return ret;

}


#endif

