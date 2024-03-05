/*
 * PinTest.c
 *
 *  Created on: 26-Feb-2013
 *      Author: Akhil Piplani
 *  Modified on: 5/10/2020
 *  	Author: Owain Barker
 */

#include <at91/boards/ISIS_OBC_G20/board.h>
#include <at91/commons.h>
#include <at91/peripherals/pio/pio.h>
#include <at91/utility/trace.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <hal/boolean.h>
#include <hal/Utility/util.h>

//This is a demonstration of the GPIO pins
//The reference pins are from the J5 and J6 connectors on the daughterboard

//Typical test case, set one GPIO to output, high
//Wire the GPIO which was set to high to a GPIO you want to check as an input
//Set that GPIO to input and read in the value set on the output of the first pin

Boolean PinTest()
{
	//List of GPIO pins which can be manipulated

	Pin pPins[] = {PIN_GPIO00, PIN_GPIO01, PIN_GPIO02, PIN_GPIO03, PIN_GPIO04, PIN_GPIO05, PIN_GPIO06, PIN_GPIO07, PIN_GPIO08,
			PIN_GPIO09, PIN_GPIO10, PIN_GPIO11, PIN_GPIO12, PIN_GPIO13, PIN_GPIO14, PIN_GPIO15, PIN_GPIO16, PIN_GPIO17, PIN_GPIO18,
			PIN_GPIO19, PIN_GPIO20, PIN_GPIO21};

	int choice = 0, activePin = 0, input_output = 0, high_low = 0;
	unsigned char pinValue;

	do
	{
		printf("\n\r Select a pin to test GPIO0 - GPIO21 (0 - 21), or -1 to quit \n\r");
		while(UTIL_DbguGetIntegerMinMax(&choice, -1, 21) == 0);

		if(choice < 0)
			return TRUE;

		activePin = choice;

		printf("\n\r Configure as input (0) or output (1) \n\r");
		while(UTIL_DbguGetIntegerMinMax(&choice, 0, 1) == 0);

		input_output = (unsigned int)choice;

		switch(input_output) {
			case 0:
				//Configure the active pin as an input
				pPins[activePin].type = PIO_INPUT;
				break;
			case 1:
				//Configure as an output pin with default 0
				 pPins[activePin].type = PIO_OUTPUT_0;
				break;
			default:
				printf("\n\r Invalid choice \n\r");
				break;
		}

		//Set the pin to the desired state
		if(!PIO_Configure(&pPins[activePin], PIO_LISTSIZE(pPins[activePin])))
		{
			printf(" PinTest: Unable to configure PIO pin %d \n\r", activePin);
		}

		//If we want it to be an output, set it high or low
		if(input_output)
		{
			printf("\n\r Set Pin as low (0) or high (1) \n\r");
			while(UTIL_DbguGetIntegerMinMax(&choice, 0, 1) == 0);

			high_low = choice;

			switch(high_low) {
				case 0:
					//configure active pin as an output low
					//In the context of this program this is redundant as this was already configured as a low pin output by default
					//However clearing the pin will set the voltage low
					PIO_Clear(&pPins[activePin]);
					break;
				case 1:
					//configure active pin as an output high
					PIO_Set(&pPins[activePin]);
					break;
				default:
					printf("\n\r Invalid choice \n\r");
					break;
			}

			//Now we can measure until this pin is reset

			printf(" \n\rMeasure the pin output\n\r");
		}
		else
		{
			printf("\n\r PinTest: Waiting for an input, when ready you can press 1 to continue to read the status. \n\r");
			UTIL_DbguGetInteger(&choice);

			pinValue = PIO_Get(&pPins[activePin]);

			printf("\n\r Pin Test: The current value of GPIO%d is %u. \n\r", activePin, pinValue);
		}

		vTaskDelay(10);
	}while(choice >= 0);

	return TRUE;
}
