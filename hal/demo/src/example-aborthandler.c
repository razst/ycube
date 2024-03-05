/*
 * example-aborthandler.c
 *
 *  Created on: 5 Mar 2020
 *      Author: asau
 */

#include <stdint.h>
#include <unistd.h>
#include "at91/utility/exithandler.h"

#include "at91/commons.h"
#include "at91/boards/ISIS_OBC_G20/board.h"
#include "at91/utility/trace.h"
#include "at91/peripherals/cp15/cp15.h"

//Forward declaring some helper function
static void disableCaches();
static void print_hex(uint32_t value);

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
        unsigned int sp, unsigned int lr,
        unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3,
        unsigned int r4, unsigned int r5, unsigned int r6, unsigned int r7,
        unsigned int r8, unsigned int r9, unsigned int r10, unsigned int r11,
        unsigned int r12, unsigned int lastLR,
        unsigned int dataabort)
{
    /*
     * Here you can now process the crash and print out what you want
     * In this example we are printing out all provided values and performing a reboot
     *
     * NOTE: Please note that the code within this function should be crash proof
     * in order to avoid additional crashes.
     * Because of this, printf can not used. The write function can be used as a replacement.
     * A simple function for printing 32 bit integers to screen as HEX has been
     * added to further facilitate printing without using printf
     */

    //Dump all registers to screen
    STATIC_PRINT("Register dump at crash:\n\r");
    //Printing r0 to r14 from crashed mode
    STATIC_PRINT("\tr0:     "); print_hex(r0);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr1:     "); print_hex(r1);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr2:     "); print_hex(r2);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr3:     "); print_hex(r3);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr4:     "); print_hex(r4);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr5:     "); print_hex(r5);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr6:     "); print_hex(r6);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr7:     "); print_hex(r7);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr8:     "); print_hex(r8);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr9:     "); print_hex(r9);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr10:    "); print_hex(r10);     STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr11:    "); print_hex(r11);     STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr12:    "); print_hex(r12);     STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr13/sp: "); print_hex(sp);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr14/lr: "); print_hex(lr);      STATIC_PRINT("\n\r");

    //CPSR and SPSR gives information about the crash and current run mode
    STATIC_PRINT("\tcpsr:   "); print_hex(cpsr);    STATIC_PRINT("\n\r");
    STATIC_PRINT("\tspsr:   "); print_hex(spsr);    STATIC_PRINT("\n\r");

    //Last function call before crash. Usually not that useful, but captured
    // and printed just in case...
    STATIC_PRINT("\tlastLR: "); print_hex(lastLR);  STATIC_PRINT("\n\r\n\r");


    if (dataabort==0)
    {
        //Manually implementing the TRACE_WARNING_WP macro to avoid printf
#if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
        STATIC_PRINT("\n\r\t\t ____PREFETCH ABORT! RESTARTING____\n\r");
#endif
    }
    else
    {
        //Manually implementing the TRACE_WARNING_WP macro to avoid printf
#if (TRACE_LEVEL >= TRACE_LEVEL_WARNING)
        STATIC_PRINT("\n\r\t\t ____DATA ABORT! RESTARTING____\n\r");
#endif
    }
    CP15_Disable_AlignmentFault();
    disableCaches();
    gracefulReset();
}

/*!
 * @brief Disables the Instruction and Data caches of the ARM926EJ-S core.
 */
static void disableCaches()
{
    CP15_DisableMMU();
    CP15_Disable_D_Cache();
    CP15_Disable_I_Cache();
}

static void print_hex(uint32_t value)
{
    DBGU_PutChar('0');
    DBGU_PutChar('x');
    int8_t i;
    for (i = 7; i >= 0; i--)
    {
        uint8_t nibble = (value >> (4 * i)) & 0xF;
        if (nibble < 10)
        {
            DBGU_PutChar('0' + nibble);
        }
        else
        {
            DBGU_PutChar('A' + (nibble-10));
        }
    }
}

