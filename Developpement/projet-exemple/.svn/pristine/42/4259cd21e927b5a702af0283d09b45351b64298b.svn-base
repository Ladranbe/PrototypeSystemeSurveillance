/**
 * demo_adc.c
 *
 *  Created on: 30 octobre 2015
 *      Author: Samuel Poiraud
 */

#include "demo_audio.h"
#include "stm32f4_discovery_audio.h"
#include "string.h"
#include "usb_conf.h"

#define AUDIO_BUFFER_SIZE   8192
#define MIC_BUFFER_SIZE		1024
#if MIC_BUFFER_SIZE > AUDIO_BUFFER_SIZE
	#error "MIC_BUFFER_SIZE must be < AUDIO_BUFFER_SIZE"
#endif
	#include "ff.h"
	#include "ff_gen_drv.h"
	#include "usbh_diskio.h"
	#include "diskio.h"
#ifdef USE_HOST_MODE
	#include "usbh_msc.h"
	extern void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);
	USBH_HandleTypeDef 			hUSB_Host;
	FATFS usb_fatfs;           /* File system object for USB disk logical drive */
#endif


uint8_t pHeaderBuff[44];
//uint16_t WrBuffer[WR_BUFFER_SIZE];

//static uint16_t RecBuf[2*PCM_OUT_SIZE];
//static uint16_t InternalBuffer[INTERNAL_BUFF_SIZE];
__IO uint32_t ITCounter = 0;

/* Temporary data sample */
__IO uint32_t AUDIODataReady = 0, AUDIOBuffOffset = 0;

volatile bool_e flag_audio_in_full = FALSE;
volatile bool_e flag_audio_out_finished = FALSE;
volatile bool_e finish_play = FALSE;


static void Error_Handler(void);


typedef enum
{
	BUFFER_ID_0 = 0,
	BUFFER_ID_1,
	BUFFER_ID_NB
}buffer_id_e;

typedef struct
{
	FIL	file;
	char * path;			//Pointeur vers le chemin du fichier
	uint32_t frequency;
	uint8_t buffer[2*AUDIO_BUFFER_SIZE];
	buffer_id_e current_buffer_id;
	UINT	buffer_index;
}audio_file_t;

const char * 				default_wave_file = "exo.wav";
#ifdef USE_HOST_MODE
static char 				USBDISKPath[4];         					/* USB Host logical drive path */
static uint8_t 				USBDisk_id;	  								/* USB id */
#endif
static audio_file_t			audio_file;
volatile bool_e bTransferComplete = FALSE;
static uint8_t current_volume;

static uint16_t audio_buffers[BUFFER_ID_NB][AUDIO_BUFFER_SIZE];
/**
 * @brief 	Cette fonction est une machine a etat qui regit le comportement de la demo du rng.
 * @func 	running_e DEMO_rng_statemachine (bool_e ask_for_finish)
 * @param 	exit_asked: demande a l'application de quitter ou non
 * @return	cette fonction retourne un element de l'enumeration running_e (END_OK= l'application est quittee avec succes ou IN_PROGRESS= l'application est toujours en cours)
 */
running_e DEMO_audio_statemachine (bool_e ask_for_finish, char touch_pressed)
{
	typedef enum
	{
		INIT = 0,
		WAIT_PHYSICAL_DRIVE,
		OPEN_FILE,
		MICRO_START,
		MICRO_WAIT,
		COMPUTE,


		PLAY,
		PLAY_FILE,
		READING_NEXT_BUFFER,
		PLAYING,
		CLOSE_FILE,
		CLOSE,
		END_WITH_ERROR,

		PLAY2,
		PLAYING2

	}state_e;
	running_e ret = IN_PROGRESS;
	static state_e state = INIT;
	static state_e asked_state;
	switch(state)
	{
		case INIT:
			current_volume = 64;

			#ifdef USE_HOST_MODE
				if(FATFS_LinkDriver(&USBH_Driver, USBDISKPath) == 0)
				{	//Initialisation du module USB
						USBDisk_id = USBDISKPath[0]-'0';
						USBH_Init(&hUSB_Host, USBH_UserProcess, 0);
						USBH_RegisterClass(&hUSB_Host, USBH_MSC_CLASS);
						USBH_Start(&hUSB_Host);
				}
				printf("wait for USB key...");
				state = WAIT_PHYSICAL_DRIVE;
			#else
				printf("To play %s file, you should compile software with USE_HOST_MOST defined in the usb_conf.h file.\n", default_wave_file);
				if(BSP_AUDIO_IN_Init(DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_CHANNEL_NBR) != AUDIO_OK)
					Error_Handler();
				state = MICRO_START;
			#endif
			break;
		case WAIT_PHYSICAL_DRIVE:
			#ifdef USE_HOST_MODE
				if(USBH_MSC_IsReady(&hUSB_Host))
				{
					state = END_WITH_ERROR;	//On suppose que ca ne va pas fonctionner.
					printf("USB key detected...");
					if(disk_initialize(USBDisk_id) == RES_OK)
					{
						if(f_mount(&usb_fatfs, (TCHAR const*)USBDISKPath, 1) == FR_OK)
						{
							printf("key mounted...");
							state = OPEN_FILE;
						}
					}
					if(state == END_WITH_ERROR)
						printf("error with usb key. Check the key is <=2GB and correctly inserted.\n");
				}
			#endif
			break;
		case OPEN_FILE:
			audio_file.path = (char*)default_wave_file;
			if(f_open(&audio_file.file,audio_file.path,FA_READ) == FR_OK)
				state = PLAY;
			else
				state = END_WITH_ERROR;
			break;

		case PLAY:{
			//Les informations nécessaires à la lecture d'un fichier wave sont contenus dans les 44 premiers octets
			#define HEADER_SIZE 44
			static uint8_t header[HEADER_SIZE];
			UINT nbBytesRead;
			state = END_WITH_ERROR;

			//Lecture de l'en-tête du fichier wave
			if(f_read(&audio_file.file, header, HEADER_SIZE, &nbBytesRead) == FR_OK)
			{
				//Récupération de la fréquence codée sur quatre octets en little-endian
				audio_file.frequency = U32FROMU8(header[27], header[26], header[25], header[24]);
				//Récupération des données audio
				if(f_read(&audio_file.file, audio_file.buffer, 2*AUDIO_BUFFER_SIZE, &audio_file.buffer_index) == FR_OK)
				{
					if(audio_file.buffer_index > 0)
					{
						int j;
						//Transforme le tableau d'entier sur 8 bits en tableau d'entiers sur 16 bits
						audio_file.current_buffer_id = BUFFER_ID_0;
						for(j=0;j<audio_file.buffer_index/2;j++)
							audio_buffers[audio_file.current_buffer_id][j] = U16FROMU8(audio_file.buffer[2*j+1],audio_file.buffer[2*j]);

						if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, current_volume, audio_file.frequency) == AUDIO_OK)
						{
							printf("playing %s audio file\n Press '+'/'-' to adjust volume. \nPress 's' to stop/restart.\n",audio_file.path);
							printf("WARNING, the CS43L22 (Audio DAC with Integrated Class D Speaker Driver) share the same pin than UART6-RX. You should use UART2 (PA3) to send your commands.\n");
							if(BSP_AUDIO_OUT_Play(audio_buffers[audio_file.current_buffer_id], audio_file.buffer_index/2) == AUDIO_OK)
								state = READING_NEXT_BUFFER;
						}
					}
					else
					{
						state = CLOSE_FILE;
						asked_state = INIT;
					}
				}
			}
			break;}
		case READING_NEXT_BUFFER:{
			audio_file.current_buffer_id = (audio_file.current_buffer_id==BUFFER_ID_0)?BUFFER_ID_1:BUFFER_ID_0;
			state = END_WITH_ERROR;
			if(f_read(&audio_file.file, audio_file.buffer, 2*AUDIO_BUFFER_SIZE, &audio_file.buffer_index) == FR_OK)
			{
				if(audio_file.buffer_index > 0)
				{
					int j;
					for(j=0;j<audio_file.buffer_index/2;j++)
						audio_buffers[audio_file.current_buffer_id][j] = U16FROMU8(audio_file.buffer[2*j+1],audio_file.buffer[2*j]);
					state = PLAYING;
				}
				else
					state = CLOSE_FILE;
			}
			break;}
		case PLAYING:
			if(bTransferComplete)
			{
				bTransferComplete = FALSE;
				BSP_AUDIO_OUT_ChangeBuffer(audio_buffers[audio_file.current_buffer_id], audio_file.buffer_index/2);
				state = READING_NEXT_BUFFER;
			}
			switch(touch_pressed)
			{
				case '+':
					current_volume = (current_volume < 91)? current_volume+10 : 100;
					BSP_AUDIO_OUT_SetVolume(current_volume);
					break;
				case '-':
					current_volume = (current_volume > 9)? current_volume-10 : 0;
					BSP_AUDIO_OUT_SetVolume(current_volume);
					break;
				case 's':	//no break;
				case 'S':
					state = CLOSE_FILE;	//Fin de la lecture.
					asked_state = OPEN_FILE;	//Dans ce demo_audio, on joue le fichier en boucle...
					break;
				case 'R':	//no break;
				case 'r':
					state = CLOSE_FILE;
					asked_state = OPEN_FILE;//MICRO_START;
					break;
				default:
					break;
			}
			if(ask_for_finish)
				state = CLOSE_FILE;
			break;

		case CLOSE_FILE:
			f_close(&audio_file.file);
			BSP_AUDIO_OUT_Stop(0);
			state = asked_state;	//l'état CLOSE_FILE renvoie ensuite vers l'état qui a été demandé...
			break;

#if 0	//Code en cours de développement...
		case MICRO_START:
			AUDIODataReady = 0;
			// Start the record
			audio_file.current_buffer_id = BUFFER_ID_0;
			bTransferComplete = FALSE;
			if (BSP_AUDIO_IN_Record((uint16_t*)audio_buffers[audio_file.current_buffer_id], MIC_BUFFER_SIZE) != AUDIO_OK)
				Error_Handler();

			state = MICRO_WAIT;
			break;
		case MICRO_WAIT:
			if(bTransferComplete)
			{
				/* PDM to PCM data convert */
				BSP_AUDIO_IN_PDMToPCM((uint16_t*)audio_buffers[audio_file.current_buffer_id], (uint16_t*)&pcm_buf[0]);

				/* Copy PCM data in internal buffer */
				memcpy((uint16_t*)&WrBuffer[ITCounter * (PCM_OUT_SIZE*2)], RecBuf, PCM_OUT_SIZE*4);
			}

			while(AUDIODataReady != 2)
			{
				if(BufferCtl.offset == BUFFER_OFFSET_HALF)
				{
					/* PDM to PCM data convert */
					//BSP_AUDIO_IN_PDMToPCM((uint16_t*)&InternalBuffer[0], (uint16_t*)&RecBuf[0]);

					/* Copy PCM data in internal buffer */
					//memcpy((uint16_t*)&WrBuffer[ITCounter * (PCM_OUT_SIZE*2)], RecBuf, PCM_OUT_SIZE*4);

					BufferCtl.offset = BUFFER_OFFSET_NONE;

					if(ITCounter == (WR_BUFFER_SIZE/(PCM_OUT_SIZE*4))-1)
					{
						AUDIODataReady = 1;
						AUDIOBuffOffset = 0;
						ITCounter++;
					}
					else if(ITCounter == (WR_BUFFER_SIZE/(PCM_OUT_SIZE*2))-1)
					{
						AUDIODataReady = 2;
						AUDIOBuffOffset = WR_BUFFER_SIZE/2;
						ITCounter = 0;
					}
					else
					{
						ITCounter++;
					}

				}

				if(BufferCtl.offset == BUFFER_OFFSET_FULL)
				{
					/* PDM to PCM data convert */
					//BSP_AUDIO_IN_PDMToPCM((uint16_t*)&InternalBuffer[INTERNAL_BUFF_SIZE/2], (uint16_t*)&RecBuf[0]);

					/* Copy PCM data in internal buffer */
					//memcpy((uint16_t*)&WrBuffer[ITCounter * (PCM_OUT_SIZE*2)], RecBuf, PCM_OUT_SIZE*4);

					BufferCtl.offset = BUFFER_OFFSET_NONE;

					if(ITCounter == (WR_BUFFER_SIZE/(PCM_OUT_SIZE*4))-1)
					{
						AUDIODataReady = 1;
						AUDIOBuffOffset = 0;
						ITCounter++;
					}
					else if(ITCounter == (WR_BUFFER_SIZE/(PCM_OUT_SIZE*2))-1)
					{
						AUDIODataReady = 2;
						AUDIOBuffOffset = WR_BUFFER_SIZE/2;
						ITCounter = 0;
					}
					else
					{
						ITCounter++;
					}
				}
			}
			//else
				state = PLAY;

			if(ask_for_finish)
				state = CLOSE;
			break;
		case COMPUTE:
			/* PDM to PCM data convert */
			//BSP_AUDIO_IN_PDMToPCM(buffer, recBuf);

			/* Copy PCM data in internal buffer */
			//memcpy((uint16_t*)&WrBuffer[ITCounter * (PCM_OUT_SIZE*2)], recBuf, PCM_OUT_SIZE*4);

			state = PLAY2;
			break;
		case PLAY2:
			 /* Stop audio record */
			  if (BSP_AUDIO_IN_Stop() != AUDIO_OK)
			      Error_Handler();

			  AudioTest = 1;	/* Set variable to indicate play from record buffer */

			  BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 70, DEFAULT_AUDIO_IN_FREQ);	/* Initialize audio IN at REC_FREQ */

			  /* Set the total number of data to be played */
			  AudioTotalSize = AUDIODATA_SIZE * WR_BUFFER_SIZE;
			  /* Update the remaining number of data to be played */
			  AudioRemSize = 0;
			  /* Update the WrBuffer audio pointer position */
			 // CurrentPos = (uint16_t *)(WrBuffer);

			  /* Play the recorded buffer */
			 // BSP_AUDIO_OUT_Play(WrBuffer , AudioTotalSize);
			//BSP_AUDIO_OUT_Play(recBuf,AUDIO_BUFFER_SIZE);
			state = PLAYING2;
			break;




		case PLAY_FILE:

			break;
		case PLAYING2:
			if(finish_play)//!HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0))
			{
				finish_play = FALSE;
				/* Stop Player before close Test */
				if (BSP_AUDIO_OUT_Stop(0/*CODEC_PDWN_SW*/) != AUDIO_OK)
					Error_Handler();
				state = MICRO_START;
			}
			/*if(flag_audio_out_finished)
			{
				flag_audio_out_finished = FALSE;
				state = MICRO_START;
			}
			*/
			if(ask_for_finish)
				state = CLOSE;
			break;
		case CLOSE:
			BSP_AUDIO_IN_Stop();
			BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);
			state = INIT;
			ret = END_OK;
			break;
#endif
		case END_WITH_ERROR:
			//Hole...
			break;
		default:
			break;
	}

	#ifdef USE_HOST_MODE
		USBH_Process(&hUSB_Host);
	#endif

	return ret;
}


extern I2S_HandleTypeDef       hAudioOutI2s;
extern I2S_HandleTypeDef       hAudioInI2s;

/**
  * @brief  This function handles main I2S interrupt.
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
void I2S3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hAudioOutI2s.hdmatx);
}

/**
  * @brief  This function handles I2S IT Stream interrupt request.
  * @param  None
  * @retval None
  */
void I2S2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(hAudioInI2s.hdmarx);
}


void audio_record_finish_play(void)
{
	finish_play = TRUE;
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
	 bTransferComplete = TRUE;
}

void BSP_AUDIO_OUT_Error_CallBack(void)
{
  /* Stop the program with an infinite loop */
  Error_Handler();
}


/**
  * @brief Calculates the remaining file size and new position of the pointer.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
	bTransferComplete = TRUE;
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
 // BufferCtl.offset = BUFFER_OFFSET_HALF;
	bTransferComplete = TRUE;
}

/**
  * @brief  Audio IN Error callback function
  * @param  pData
  * @retval None
  */
void BSP_AUDIO_IN_Error_Callback(void)
{
  /* Stop the program with an infinite loop */
  Error_Handler();
}

static void Error_Handler(void)
{
  while(1)
  {
  }
}
