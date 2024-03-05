/*
 * port.h
 *
 *  Created on: 4 May 2020
 *      Author: asau
 */

#ifndef INCLUDE_FREERTOS_PORTABLE_GCC_ARM9_AT91SAM9G20_PORT_H_
#define INCLUDE_FREERTOS_PORTABLE_GCC_ARM9_AT91SAM9G20_PORT_H_

/*
 *
 */

/*
 * Assigns the DBGU interrupt handler to use
 * @note: This is NOT part of FreeRTOS, but added to allow HAL to hook into vPortTickISR
 */
void portISR_SetDBUGHandler(void (*funcPtr)(void));
/*
 * Assigns the RTT alarm interrupt handler to use
 * @note: This is NOT part of FreeRTOS, but added to allow HAL to hook into vPortTickISR
 */
void portISR_SetRTTHandler(void (*funcPtr)(void));

#endif /* INCLUDE_FREERTOS_PORTABLE_GCC_ARM9_AT91SAM9G20_PORT_H_ */
