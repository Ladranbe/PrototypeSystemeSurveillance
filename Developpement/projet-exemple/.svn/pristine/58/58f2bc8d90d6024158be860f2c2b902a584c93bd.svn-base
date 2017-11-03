/*
 * demo_webserver.c
 *
 *  Created on: 2 mars 2015
 *      Author: Nirgal
 */
#include <stdint.h>
#include "macro_types.h"
#include "demo_webserver.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4_sys.h"
#include "stm32f4_gpio.h"
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "diskio.h"
#include "lcd_display_form_and_text.h"
#include "demo_sd.h"
#include "tm_stm32f4_ethernet.h"
#include "stm32f4_rtc.h"

volatile uint32_t time_ms = 0;
volatile bool_e flag_50ms = FALSE;
volatile bool_e webserver_is_on = FALSE;

extern FATFS SDFatFs;	//d�finit dans demo_sd...
extern Diskio_drvTypeDef  SD_Driver;
extern char SDPath[4]; // SD card logical drive path
/*
 * Pour utiliser le module Webserver :
 * 1- activez la compilation de lib/webserver (bouton droit -> ressource configuration -> exclude from build -> d�cocher)
 * 2- activez la macro USE_WEBSERVER ci-dessous
 *
 * Pour ne pas utiliser le module webserver (et surtout ne pas subir sa longue compilation)
 * 1- d�sactiver la compilation de lib/webserver (bouton droit -> ressource configuration -> exclude from build -> cocher)
 * 2- d�sactivez la macro USE_WEBSERVER ci-dessous
 */
#define USE_WEBSERVER



const uint8_t mac_addr[6] = {0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
const uint8_t ip_addr[4] = {192, 168, 10, 210};
const uint8_t gateway[4] = {192, 168, 10, 50};
const uint8_t netmask[4] = {255, 255, 255, 0};

/* Set SSI tags for handling variables */
static TM_ETHERNET_SSI_t SSI_Tags[] = {
	"led1_s", /* Tag 0 = led1 status */
	"led2_s", /* Tag 1 = led2 status */
	"led3_s", /* Tag 2 = led3 status */
	"led4_s", /* Tag 3 = led4 status */
	"srv_adr",/* Tag 4 = server address */
	"clt_a_c",/* Tag 5 = client all connections */
	"clt_s_c",/* Tag 6 = client successfull connections */
	"clt_per",/* Tag 7 = client percentage */
	"clt_tx", /* Tag 8 = client TX bytes */
	"clt_rx", /* Tag 9 = client RX bytes */
	"srv_c",  /* Tag 10 = server all connections */
	"srv_tx", /* Tag 11 = server TX bytes */
	"srv_rx", /* Tag 12 = server RX bytes */
	"mac_adr",/* Tag 13 = MAC address */
	"gateway",/* Tag 14 = gateway */
	"netmask",/* Tag 15 = netmask */
	"link",   /* Tag 16 = link status */
	"duplex", /* Tag 17 = duplex status */
	"hardware",/* Tag 18 = hardware where code is running */
	"rtc_time",/* Tag 19 = current RTC time */
	"compiled",/* Tag 20 = compiled date and time */
};
#define SSI_TAGS_NB	21

/* LED CGI handler */
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

/* CGI call table, only one CGI used */
TM_ETHERNET_CGI_t CGI_Handlers[] = {
	{"/ledaction.cgi", LEDS_CGI_Handler} /* LEDS_CGI_Handler will be called when user connects to "/ledaction.cgi" URL */
};
#define CGI_HANDLED_NB	1

//Fonction pour tester la librairie webserver
running_e DEMO_webserver(bool_e ask_for_finish, bool_e display_on_lcd)
{
	typedef enum
	{
		INIT = 0,
		DISPLAY,
		RUNNING,
		CLOSE
	}state_e;

	static state_e state = INIT;
	static bool_e sd_is_ready = FALSE;
	running_e ret;
	ret = IN_PROGRESS;

	switch(state)
	{
		case INIT:
			#ifdef USE_WEBSERVER
				BSP_SD_Init();
				sd_is_ready = FALSE;
				if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
				{
					if(disk_initialize(PD_SD) == 0)
					{
						f_mount(&SDFatFs, "0:", 1);
						sd_is_ready = TRUE;
					}
				}

				/* Display to user */
				printf("Webserver starting..\n");

				/* Initialize ethernet peripheral */
				/* All parameters NULL, default options for MAC, static IP, gateway and netmask will be used */
				/* They are defined in tm_stm32f4_ethernet.h file */
				if (TM_ETHERNET_Init((uint8_t *)mac_addr, (uint8_t *)ip_addr, (uint8_t *)gateway, (uint8_t *)netmask) == TM_ETHERNET_Result_Ok)
				/* Successfully initialized */

				/* Initialize ethernet server if you want use it, server port 80 */
				TM_ETHERNETSERVER_Enable(80);

				/* Set SSI tags, we have 21 SSI tags */
				TM_ETHERNETSERVER_SetSSITags(SSI_Tags, SSI_TAGS_NB);

				/* Set CGI tags, we have 1 CGI handler, for leds only */
				TM_ETHERNETSERVER_SetCGIHandlers(CGI_Handlers, CGI_HANDLED_NB);

				webserver_is_on = TRUE;
			#endif
				state = (display_on_lcd)?DISPLAY:RUNNING;

			break;
		case DISPLAY:{
			char buf[50];
			LCD_SetFont(&Font8x8);
			LCD_Clear(LCD_COLOR_WHITE);
			#ifdef USE_WEBSERVER
				if(sd_is_ready)
					LCD_DisplayStringLine(LINE(2),COLUMN(0),(uint8_t *)"Webserver pret..."						,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				else
					LCD_DisplayStringLine(LINE(2),COLUMN(0),(uint8_t *)"Webserver : insert SD Card + reset!"	,LCD_COLOR_RED,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);

				LCD_DisplayStringLine(LINE(4),COLUMN(0),(uint8_t *)"Assurez vous d'avoir insere une carte SD"	,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				LCD_DisplayStringLine(LINE(6),COLUMN(0),(uint8_t *)"contenant un fichier index.htm"				,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				LCD_DisplayStringLine(LINE(8),COLUMN(0),(uint8_t *)"et d'etre connecte au meme reseau"			,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);

				sprintf(buf, "Server IP (fixed) : %d.%d.%d.%d", TM_ETHERNET_GetLocalIP(0), TM_ETHERNET_GetLocalIP(1), TM_ETHERNET_GetLocalIP(2), TM_ETHERNET_GetLocalIP(3));
				LCD_DisplayStringLine(LINE(12),COLUMN(0),(uint8_t *)buf,LCD_COLOR_BLACK,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			#else
				LCD_DisplayStringLine(LINE(2),COLUMN(0),(uint8_t *)"Module Webserver non compile."				,LCD_COLOR_RED,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				LCD_DisplayStringLine(LINE(4),COLUMN(0),(uint8_t *)"-Definissez la macro USE_WEBSERVER"			,LCD_COLOR_RED,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				LCD_DisplayStringLine(LINE(6),COLUMN(0),(uint8_t *)"  dans demo_webserver.c"						,LCD_COLOR_RED,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
				LCD_DisplayStringLine(LINE(8),COLUMN(0),(uint8_t *)"-activez la compilation de lib/webserver"	,LCD_COLOR_RED,LCD_COLOR_WHITE,LCD_DISPLAY_ON_UART);
			#endif
			state = RUNNING;
			break;}
		case RUNNING:
			#ifdef USE_WEBSERVER
				TM_ETHERNET_Update();
			#endif
			if(ask_for_finish)
				state = CLOSE;
			break;
		case CLOSE:
			webserver_is_on = FALSE;
			ret = END_OK;
			state = DISPLAY;	//On ne retourne pas en INIT. (les fonctions de fermeture n'�tant pas compl�t�es).
			break;
		default:
			break;
	}

	return ret;
}

#ifdef USE_WEBSERVER
void DEMO_webserver_process_1ms(void)
{
	if(webserver_is_on)
	{
		TM_ETHERNET_TimeUpdate(1);
		time_ms++;		//Jamais remis � 0. (le d�bordement survient tout les 49,7 jours).
		if(! (time_ms%50) )
			flag_50ms = TRUE;
	}
}





/* Handle CGI request for LEDS */
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
	uint8_t i;

	/* This function handles request like one below: */
	/* /ledaction.cgi?ledtoggle=1&ledoff=2 */
	/* This will toggle LED 1 and turn off LED 2 */

	/* Callback */
	if (iIndex == 0) {
		/* Go through all params */
		for (i = 0; i < iNumParams; i++) {
			/* If current pair = ledtoggle=someled */
			if (strstr(pcParam[i], "ledtoggle")) {
				/* Switch first character */
				switch (pcValue[i][0]) {
					case '1':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_12));	//LED_GREEN
						break;
					case '2':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_13));	//LED_ORANGE
						break;
					case '3':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_14));	//LED_RED
						break;
					case '4':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,!HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_15));	//LED_BLUE
						break;
					default:
						break;
				}
			} else if (strstr(pcParam[i], "ledon")) {
				switch (pcValue[i][0]) {
					case '1':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,1);	//LED_GREEN
						break;
					case '2':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,1);	//LED_ORANGE
						break;
					case '3':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,1);	//LED_RED
						break;
					case '4':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,1);	//LED_BLUE
						break;
					default:
						break;
				}
			} else if (strstr(pcParam[i], "ledoff")) {
				switch (pcValue[i][0]) {
					case '1':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,0);	//LED_GREEN
						break;
					case '2':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,0);	//LED_ORANGE
						break;
					case '3':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,0);	//LED_RED
						break;
					case '4':
						HAL_GPIO_WritePin(GPIOD,GPIO_PIN_15,0);	//LED_BLUE
						break;
					default:
						break;
				}
			}
		}
	}

	/* Return URL to be used after call */
	return "/index.ssi";
}

/* SSI server callback, always is called this callback */
bool_e TM_ETHERNETSERVER_SSICallback(char * tag_name, char *pcInsert, int iInsertLen, uint16_t * iInsertedLen)
{
	uint8_t loop;
	bool_e ret;
	ret = FALSE;	//default: tag found

	for(loop = 0; loop < SSI_TAGS_NB; loop++)
	{
		if(strcmp(tag_name, SSI_Tags[loop]) == 0)
		{
			ret = TRUE;
			const char * green_on = "<span class=\"green\">On</span>";
			const char * red_off = "<span class=\"red\">Off</span>";
			switch(loop)
			{
				case 0:	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_12))	sprintf(pcInsert, (char*)green_on);	else sprintf(pcInsert, (char*)red_off); break;
				case 1:	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_13))	sprintf(pcInsert, (char*)green_on);	else sprintf(pcInsert, (char*)red_off); break;
				case 2:	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_14))	sprintf(pcInsert, (char*)green_on);	else sprintf(pcInsert, (char*)red_off); break;
				case 3:	if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_15))	sprintf(pcInsert, (char*)green_on);	else sprintf(pcInsert, (char*)red_off); break;
				case 4:	/* #serv_adr tag*/	sprintf(pcInsert, "%d.%d.%d.%d", TM_ETHERNET_GetLocalIP(0), TM_ETHERNET_GetLocalIP(1), TM_ETHERNET_GetLocalIP(2), TM_ETHERNET_GetLocalIP(3));			break;
				case 5:	/* #clt_a_c tag */	sprintf(pcInsert, "%lu", TM_ETHERNETCLIENT_GetConnectionsCount());				break;
				case 6:	/* #clt_s_c tag */	sprintf(pcInsert, "%lu", TM_ETHERNETCLIENT_GetSuccessfullConnectionsCount());	break;
				case 7:	/* #clt_per tag */
					if (TM_ETHERNETCLIENT_GetConnectionsCount() == 0)
						strcpy(pcInsert, "0 %");
					else
						sprintf(pcInsert, "%f %%", (float)TM_ETHERNETCLIENT_GetSuccessfullConnectionsCount() / (float)TM_ETHERNETCLIENT_GetConnectionsCount() * 100);
					break;
				case 8:	/* #clt_tx tag */	sprintf(pcInsert, "%llu", TM_ETHERNETCLIENT_GetTXBytes());						break;
				case 9:	/* #clt_rx tag */	sprintf(pcInsert, "%llu", TM_ETHERNETCLIENT_GetRXBytes());						break;
				case 10:/* #srv_c tag */	sprintf(pcInsert, "%lu", TM_ETHERNETSERVER_GetConnectionsCount());				break;
				case 11:/* #srv_tx tag */	sprintf(pcInsert, "%llu", TM_ETHERNETSERVER_GetTXBytes());						break;
				case 12:/* #srv_rx tag */	sprintf(pcInsert, "%llu", TM_ETHERNETSERVER_GetRXBytes());						break;
				case 13:/* #mac_adr */
					sprintf(pcInsert, "%02X-%02X-%02X-%02X-%02X-%02X",
						TM_ETHERNET_GetMACAddr(0),
						TM_ETHERNET_GetMACAddr(1),
						TM_ETHERNET_GetMACAddr(2),
						TM_ETHERNET_GetMACAddr(3),
						TM_ETHERNET_GetMACAddr(4),
						TM_ETHERNET_GetMACAddr(5)
					);
					break;
				case 14:/* #gateway */
					sprintf(pcInsert, "%d.%d.%d.%d",
						TM_ETHERNET_GetGateway(0),
						TM_ETHERNET_GetGateway(1),
						TM_ETHERNET_GetGateway(2),
						TM_ETHERNET_GetGateway(3)
					);
					break;
				case 15:/* #netmask */
					sprintf(pcInsert, "%d.%d.%d.%d",
						TM_ETHERNET_GetNetmask(0),
						TM_ETHERNET_GetNetmask(1),
						TM_ETHERNET_GetNetmask(2),
						TM_ETHERNET_GetNetmask(3)
					);
					break;
				case 16:/* #link */
					if (TM_ETHERNET_Is100M()) {
						strcpy(pcInsert, "100Mbit");
					} else {
						strcpy(pcInsert, "10Mbit");
					}
					break;
				case 17:/* #duplex */
					if (TM_ETHERNET_IsFullDuplex()) {
						strcpy(pcInsert, "Full");
					} else {
						strcpy(pcInsert, "Half");
					}
					break;
				case 18:/* #hardware */
					strcpy(pcInsert, "STM32F4-Discovery");
					break;
				case 19:{
					RTC_TimeTypeDef time;
					RTC_DateTypeDef date;
					RTC_get_time_and_date(&time, &date);

					sprintf(pcInsert, "%04d-%02d-%02d %02d:%02d:%02d",
							date.Year + 2000,
							date.Month,
							date.Date,
							time.Hours,
							time.Minutes,
							time.Seconds
					);
					break;}
				case 20:/* #compiled */
					strcpy(pcInsert, __DATE__ " at " __TIME__);
					break;
				default:
				/* No valid tag */
					ret = FALSE;
				return 0;
			}

		}
	}
	/* Return number of characters written in buffer */
	if(ret)
		*iInsertedLen = strlen(pcInsert);
	return ret;
}

void TM_ETHERNET_IPIsSetCallback(uint8_t ip_addr1, uint8_t ip_addr2, uint8_t ip_addr3, uint8_t ip_addr4, uint8_t dhcp)
{

	/* Print MAC address to user */
		printf("MAC: %02X-%02X-%02X-%02X-%02X-%02X\n",
			TM_ETHERNET_GetMACAddr(0),
			TM_ETHERNET_GetMACAddr(1),
			TM_ETHERNET_GetMACAddr(2),
			TM_ETHERNET_GetMACAddr(3),
			TM_ETHERNET_GetMACAddr(4),
			TM_ETHERNET_GetMACAddr(5)
		);

	/* Called when we have valid IP, it might be static or DHCP */
	if (dhcp) {
		/* IP set with DHCP */
		printf("IP: %d.%d.%d.%d assigned by DHCP server\n", ip_addr1, ip_addr2, ip_addr3, ip_addr4);
	} else {
		/* Static IP */
		printf("IP: %d.%d.%d.%d; STATIC IP used\n", ip_addr1, ip_addr2, ip_addr3, ip_addr4);
	}

	/* Print netmask to user */
	printf("Netmask: %d.%d.%d.%d\n",
		TM_ETHERNET_GetNetmask(0),
		TM_ETHERNET_GetNetmask(1),
		TM_ETHERNET_GetNetmask(2),
		TM_ETHERNET_GetNetmask(3)
	);
	/* Print gateway to user */
	printf("Gateway: %d.%d.%d.%d\n",
		TM_ETHERNET_GetGateway(0),
		TM_ETHERNET_GetGateway(1),
		TM_ETHERNET_GetGateway(2),
		TM_ETHERNET_GetGateway(3)
	);
	/* Print 100M link status, 1 = 100M, 0 = 10M */
	printf("Link 100M: %d\n", TM_ETHERNET.speed_100m);
	/* Print duplex status: 1 = Full, 0 = Half */
	printf("Full duplex: %d\n", TM_ETHERNET.full_duplex);
}

void TM_ETHERNET_DHCPStartCallback(void) {
	/* Called when has DHCP started with getting IP address */
	printf("Trying to get IP address via DHCP\n");
}

void TM_ETHERNET_LinkIsDownCallback(void) {
	/* This function will be called when ethernet cable will not be plugged */
	/* It will also be called on initialization if connection is not detected */

	/* Print to user */
	printf("Link is down, do you have connected to your modem/router?");
}

void TM_ETHERNET_LinkIsUpCallback(void) {
	/* Cable has been plugged in back, link is detected */
	/* I suggest you that you reboot MCU here */
	/* Do important stuff before */

	/* Print to user */
	printf("Link is up back\n");
}


/* Client is connected */
uint8_t TM_ETHERNETSERVER_ClientConnectedCallback(struct tcp_pcb *pcb) {
	struct ip_addr ip;
	/* Fill bad IP */
	IP4_ADDR(&ip, 84, 12, 16, 46);

	/* Check IP address */
	if (pcb->remote_ip.addr == ip.addr) {
		/* Print to user */
		printf("User with bad IP was trying to access to website\n");
		/* Disable access, show error page */
		return 0;
	}
	/* Print to user */
	printf("Connection allowed\n");
	/* Allow access to others */
	return 1;
}

void TM_ETHERNETSERVER_ClientDisconnectedCallback(void) {
	/* Print to user */
	printf("Client disconnected\n");
}



#endif
