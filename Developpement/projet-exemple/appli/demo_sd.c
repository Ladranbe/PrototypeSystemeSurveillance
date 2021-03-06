/*
 * demo_sd.c
 *
 *  Created on: 2 mars 2015
 *      Author: Nirgal
 */
#include <stdint.h>

#include "demo_sd.h"
#include "ff.h"
#include "lcd_display_form_and_text.h"
#include "diskio.h"
#include "sd_diskio.h"
#include "stm32f4_sd.h"
#include <string.h>
#include "stm32f4_gpio.h"


const char * datas_to_write = "test_datas\n";
const char * path = "0:test.txt";


char SDPath[4]; /* SD card logical drive path */

//Attention! La couleur de fond definie est le blanc, pensez-y lorsque vous choisirez la couleur du texte 
#define display(str, color)	LCD_DisplayStringLine(LINE(lcd_index),COLUMN(0),(uint8_t *)str,color, LCD_COLOR_WHITE, LCD_NO_DISPLAY_ON_UART); lcd_index=(lcd_index<20)?lcd_index+1:0


FATFS SDFatFs;
//Fonction pour tester la librairie d'acces a la carte SD ou a une cle USB...
running_e DEMO_sd_state_machine(bool_e ask_for_finish)
{
	typedef enum
	{
		INIT = 0,
		MOUNT,
		WRITE_TEST_FILE,
		READ_TEST_FILE,
		DELETE_TEST_FILE,
		END_WITH_ERROR,
		IDLE
	}state_e;

	static state_e state = INIT;
	static uint8_t lcd_index = 0;
	FRESULT res = FR_OK;
	static FIL file;
	running_e ret;
	ret = IN_PROGRESS;


	switch(state)
	{
		case INIT:
			lcd_index = 0;
			LCD_Clear(LCD_COLOR_WHITE);
			LCD_SetFont(&Font8x12);

			if(BSP_SD_Init() == MSD_OK)///*disk_initialize(PD_SD) == 0*/)
			{
				display("SD Card : module initialized", LCD_COLOR_BLACK);
				state = MOUNT;
			}
			else
			{
				display("SD Card : FAIL initialized", LCD_COLOR_RED);
				debug_printf("Fail to initialize disk\n");
				state = END_WITH_ERROR;
			}
			break;
		case MOUNT:{
			FATFS * fs;
			Uint32 p2;
			state = END_WITH_ERROR;	//On fait la supposition que ça ne va pas marcher.... si tout se passe bien, on ira faire la suite.
			if(FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
			{
				display("SD Card : NOT mounted", LCD_COLOR_GREEN);
			}
			else
			{
				res = f_mount(&SDFatFs, (TCHAR const*)SDPath, 0);
				if(res == FR_OK)
				{
					res = f_getfree("", (DWORD*)&p2, &fs);
					//res = f_mkfs((TCHAR const*)SDPath, 0, 0);

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
						display("SD Card : mounted", LCD_COLOR_BLACK);
						state = WRITE_TEST_FILE;
					}
				}
				if(res != FR_OK)
					verbose_fresult(res);
			}
			break;}
		case WRITE_TEST_FILE:{
			UINT nb_to_write;
			UINT nb_written;

			res = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_ALWAYS);
			if(res == FR_OK)
			{
				debug_printf("File %s opened\n",path);
				display("Testfile opened", LCD_COLOR_BLACK);
				nb_to_write = strlen((char *)datas_to_write);
				res = f_write(&file,datas_to_write,nb_to_write,&nb_written);
				if(res == FR_OK && nb_to_write == nb_written)
				{
					display("Data wrote in testfile", LCD_COLOR_BLACK);
					debug_printf("datas wrote in file\n");
				}
				else
				{
					display("FAIL to write in testfile", LCD_COLOR_RED);
					debug_printf("Fail to write datas in file\n");
				}
				f_close(&file);
			}
			else
			{
				display("FAIL to open Testfile", LCD_COLOR_RED);
				debug_printf("Fail to open file %s\n",path);
			}

			state = READ_TEST_FILE;
			break;}
		case READ_TEST_FILE:{
			#define READ_BUFFER_SIZE	32
			UINT nb_read = 0;
			uint8_t datas_read[READ_BUFFER_SIZE];
			res = f_open(&file, path, FA_READ);
			if(res == FR_OK)
			{
				res = f_read(&file,datas_read,READ_BUFFER_SIZE,&nb_read);
				datas_read[nb_read] = '\0';	//Pour utiliser strcmp...
				if(strcmp((char*)datas_read,(char*)datas_to_write) == 0)
				{
					display("Correct data read", LCD_COLOR_BLACK);
					debug_printf("Correct data read\n");
				}
				else
				{
					display("Bad datas read from testfile", LCD_COLOR_RED);
					debug_printf("Bad data read : %d datas : %s\n",nb_read,datas_read);
				}
				f_close(&file);
			}
			state = DELETE_TEST_FILE;
			break;}
		case DELETE_TEST_FILE:
			res = f_unlink(path);
			if(res == FR_OK)
			{
				display("Testfile deleted", LCD_COLOR_BLACK);
				debug_printf("Test File deleted : ok\n");
			}
			else
			{
				display("Error deleting testfile", LCD_COLOR_RED);
				debug_printf("Error deleting test file\n");
			}
			state = IDLE;
			break;

		case END_WITH_ERROR:
			debug_printf("SD Card : error\n");
			display("SD Card : ERROR", LCD_COLOR_RED);
			state = IDLE;
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

	if(res != FR_OK)
		verbose_fresult(res);
	return ret;
}





void verbose_fresult (FRESULT rc)
{
	const char *p;
	static const char str[] =
		"OK\0" "NOT_READY\0" "NO_FILE\0" "FR_NO_PATH\0" "INVALID_NAME\0" "INVALID_DRIVE\0"
		"DENIED\0" "EXIST\0" "RW_ERROR\0" "WRITE_PROTECTED\0" "NOT_ENABLED\0"
		"NO_FILESYSTEM\0" "INVALID_OBJECT\0" "MKFS_ABORTED\0";
	FRESULT i;

	for (p = str, i = 0; i != rc && *p; i++) {
		while(*p++);
	}
	debug_printf("rc=%u FR_%s\n", (UINT)rc, p);
}



void SDIO_IRQHandler(void)
{
	BSP_SD_IRQHandler();
}

void DMA2_Stream3_IRQHandler(void)
{
	BSP_SD_DMA_Rx_IRQHandler();
}

void DMA2_Stream6_IRQHandler(void)
{
	BSP_SD_DMA_Tx_IRQHandler();
}


