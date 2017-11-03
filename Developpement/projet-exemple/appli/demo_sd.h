/*
 * demo_sd.h
 *
 *  Created on: 2 mars 2015
 *      Author: Nirgal
 */

#ifndef DEMO_SD_H_
#define DEMO_SD_H_
#include "ff.h"
#include "macro_types.h"


#define PD_SD 0

#define SD_SDIO_DMA_STREAM3	          3
//#define SD_SDIO_DMA_STREAM6           6
#ifdef SD_SDIO_DMA_STREAM3
 #define SD_SDIO_DMA_IRQn              DMA2_Stream3_IRQn
 #define SD_SDIO_DMA_IRQHANDLER        DMA2_Stream3_IRQHandler 
#elif defined SD_SDIO_DMA_STREAM6
 #define SD_SDIO_DMA_IRQn              DMA2_Stream6_IRQn
 #define SD_SDIO_DMA_IRQHANDLER        DMA2_Stream6_IRQHandler
#endif /* SD_SDIO_DMA_STREAM3 */

running_e DEMO_sd_state_machine(bool_e ask_for_finish);

void verbose_fresult (FRESULT rc);
void DEMO_SD_SDCard_Init(void);
void DEMO_SD_NVIC_Configuration(void);

#endif /* DEMO_SD_H_ */
