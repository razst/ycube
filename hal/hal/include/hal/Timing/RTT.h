/*!
 * @file	RTT.h
 * @brief	Exposes the functionality of the RTT (Real Time Timer) of the AT91SAM9G20 microcontroller.
 * @author	Akhil Piplani
 */

#ifndef RTT_H_
#define RTT_H_

/*!
 * @brief Sets up the real time timer's counter to increment every second
 * (resetting it in the process).
 */
void RTT_start(void);

/*!
 * Returns the seconds at the RTT.
 * @return Seconds at the RTT.
 */
unsigned int RTT_GetTime(void);

/*!
 * Returns the status register value of the RTT.
 * @return The status register value of the RTT.
 */
unsigned int RTT_GetStatus(void);

/*!
 * Sets a future time to trigger an alarm/interrupt on.
 */
void RTT_SetAlarm(unsigned int epoch);

/*!
 * Returns the current set RTT alarm in epoch.
 * @return 0 if alarm is not set, non zero if RTT alarm is set
 */
unsigned int RTT_GetAlarm(void);

/*!
 * Sets the interrupt callback used for RTT alarms
 */
void RTT_SetAlarmInterrupt(void (*funcPtr)(void));

/*!
 * @brief Checks if the time at the RTT is changing.
 * This function should only be used for tests. Never use it during flight.
 * @warning Takes a little over 2 seconds!!
 * @return -1 if RTT time doesn't seem to be changing, 0 otherwise.
 */
int RTT_test(void);

#endif /* RTT_H_ */
