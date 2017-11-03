/*
 * Cet exemple démontre une utilisation du driver du clavier matriciel
 * 		On y trouve :
 * 			- appel à l'initialisation
 * 			- Consultation du clavier
 * 			- Affichage des touches pressées sur la liaison série
 *
 * 	Pour utiliser cet exemple :
 * 		1- Configurer le pilote dans matrix_keyboard.h avec les bonnes valeurs des ports et des pins utilisés.
 * 		2- Assurer vous d'appeler périodiquement (toutes les ms exactement) la fonction demo_matrix_keyboard_process_1ms().
 * 			Cet appel peut s'effectuer en tâche de fond ou en interruption.
 * 		3- Appeler régulièrement dans la tâche de fond la fonction demo_matrix_keyboard_process_main().
 */

#include <stdio.h>
#include <stdlib.h>
#include "demo_matrix_keyboard.h"
#include "matrix_keyboard.h"


//Les positions des touches sur ce clavier sont probablement fausses...
//	mais elles permettent de montrer l'utilisation de la fonction KEYBOARD_init(const char * new_keyboard_keys);
const char custom_keyboard[16] =  {
										'0','4','8','C',
										'1','5','9','D',
										'2','6','A','E',
										'3','7','B','F' };
const char custom_keyboard_12_touchs[16]  =  {
										'1','2','3','X',
										'4','5','6','X',
										'7','8','9','X',
										'*','0','#','X' };

static volatile uint32_t t = 0;

//Cette fonction doit être appelée dans la boucle de tâche de fond
running_e DEMO_matrix_keyboard_process_main (bool_e ask_for_finish)
{
	typedef enum
	{
		INIT = 0,
		RUN
	}state_e;
	static state_e state = INIT;
	static char previous_key = 0;
	running_e ret;
	char current_key;

	ret = IN_PROGRESS;
	switch(state)
	{
		case INIT:
			KEYBOARD_init(NULL);						//Initialisation du clavier avec le clavier par défaut
			//KEYBOARD_init(custom_keyboard);			//Initialisation du clavier avec un clavier personnalisé
			KEYBOARD_init(custom_keyboard_12_touchs);	//Initialisation du clavier avec un clavier personnalisé 12 touches
			printf("To run this demo, you should plug a matrix keyboard on the right ports. See matrix_keyboard.h\n");
			printf("Warning: the defaults ports used by this drivers are not compatible with the presence of SD card!\n");
			state = RUN;
			break;
		case RUN:
			if(!t)	//A chaque fois que t vaut 0 (toutes les 10ms)...
			{
				t = 10;							//[ms] On recharge le chronomètre t pour 10ms...
				current_key = KEYBOARD_get_key();
				if(current_key != previous_key)
				{
					switch(current_key)
					{
						case NO_KEY:											break;
						case MANY_KEYS:	printf("Many keys pressed\n");			break;
						default:		printf("%c pressed\n", current_key);	break;
					}
				}
				previous_key = current_key;
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
void DEMO_matrix_keyboard_process_1ms(void)
{
	if(t)		//Si le chronomètre est "chargé", on décompte... (comme un minuteur de cuisine !)
		t--;
}





