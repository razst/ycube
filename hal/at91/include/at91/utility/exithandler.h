/*!
 * @file	exithandler.h
 * @brief	Driver for reading Analog data (including OBC board temperature) using the ADC.
 * @author	Akhil Piplani
 * @date    Jun 22, 2011
 */

#ifndef EXITHANDLER_H_
#define EXITHANDLER_H_

//Subtracting 1 from size to avoid printing null characters
#define STATIC_PRINT(txt) write(1, txt, sizeof(txt)-1)

/*!
 * @brief Performs any housekeeping needed before reset and then calls the
 * gracefulReset function. These two functions are not merged to save space
 * on the SRAM0 by placing a small function there.
 */
void restart();


/*!
 * This is the handler that will be called when a crash occurs.
 *
 * r0 to r12, sp and lr are the registers with the values captured at crash
 *
 * lastLR can contain the pointer to the last function call. (Is often a garbage value)
 *
 * cpsr and spsr are their respective values captured at crash. These can be used
 * to extract information about the crash. See ARM926EJ-S Technical Reference Manual
 * for more information
 *
 * dataabort is 0 for prefetch aborts and 1 for data abort. This is useful to be
 * able to correctly find the crash location
 */
void handleAbort(unsigned int cpsr, unsigned int spsr,
        unsigned int spCaller, unsigned int lr,
        unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3,
        unsigned int r4, unsigned int r5, unsigned int r6, unsigned int r7,
        unsigned int r8, unsigned int r9, unsigned int r10, unsigned int r11,
        unsigned int r12, unsigned int lastLR,
        unsigned int dataabort);

/*!
 * @brief Resets the G20 gracefully by first powering down the SDRAM.
 *
 * This function must be located in the SRAM0 because it shuts down the SDRAM!
 * This is done by placing it in the sramfunc section.
 */
void gracefulReset() __attribute__ ((long_call, section (".sramfunc")));

#endif /* EXITHANDLER_H_ */
