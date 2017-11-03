/*
 * Cet exemple démontre une utilisation du driver de l'écran LCD 2x16.
 * 		On y trouve :
 * 			- appel à l'initialisation
 * 			- Affichage de caractères à l'écran
 * 			- Affichage de chaîne de caractères
 * 			- Affichage de chaîne de caractères formattée
 * 			- Déplacement du curseur d'écriture
 *
 * 	Pour utiliser cet exemple :
 * 		1- Configurer le pilote dans lcd2x16.h avec les bonnes valeurs des ports et des pins utilisés.
 * 		2- Assurer vous d'appeler périodiquement (toutes les ms exactement) la fonction demo_lcd_process_1ms().
 * 			Cet appel peut s'effectuer en tâche de fond ou en interruption.
 * 		3- Appeler régulièrement dans la tâche de fond la fonction demo_lcd_process_main().
 */

#include <stdio.h>
#include <stdlib.h>
#include "demo_lcd2x16.h"
#include "lcd2x16.h"


static volatile uint32_t t = 0;

//Cette fonction doit être appelée dans la boucle de tâche de fond
running_e DEMO_lcd2x16_process_main (bool_e ask_for_finish)
{
	typedef enum
	{
		INIT = 0,
		RUN,
		CLOSE
	}state_e;
	static uint16_t counter = 0;
	static state_e state = INIT;
	running_e ret;
	ret = IN_PROGRESS;

	switch(state)
	{
		case INIT:
			printf("To run this demo, you should plug a lcd2x16 on the right ports. See lcd2x16.h\n");
			LCD2X16_init();				//Initialisation du LCD2X16
			LCD2X16_setCursor(3,1);		//curseur sur le 3ème caractère de la 1ère ligne

			LCD2X16_putChar('c');		//Affichage d'un caractère
			LCD2X16_putChar('h');
			LCD2X16_putChar('a');
			LCD2X16_putChar('r');
			LCD2X16_putChar(' ');

			LCD2X16_printf("string");	//Affichage d'une chaîne de caractères

			state = RUN;
			break;
		case RUN:
			if(!t)	//A chaque fois que t vaut 0 (toutes les 500ms)...
			{
				t = 500;						//[ms] On recharge le chronomètre t pour 500ms...
				LCD2X16_setCursor(1,2);			//1er caractère de la 2ème ligne
				counter++;						//On incrémente le compteur static
				LCD2X16_putChar('0'+counter%10);//Affichage de counter modulo 10
				LCD2X16_printf("%4x",counter);	//Affichage de counter sur 16 bits, en hexadécimal
			}
			if(ask_for_finish)
			{
				state = INIT;
				ret = END_OK;
			}
			break;
		default:
			break;
	}
	return ret;
}

//Cette fonction doit être appelée toutes les ms.
void DEMO_lcd2x16_process_1ms(void)
{
	if(t)		//Si le chronomètre est "chargé", on décompte... (comme un minuteur de cuisine !)
		t--;
	LCD2X16_process_1ms();	//On respecte le contrat pour que le pilote fonctionne correctement.
}





