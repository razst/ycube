/*
 * ExitHandler.c
 *
 *  Created on: Jun 22, 2011
 *      Author: Akhil
 */

#include <stdint.h>
#include <unistd.h>
#include "at91/utility/exithandler.h"

#include "at91/commons.h"
#include "at91/boards/ISIS_OBC_G20/board.h"
#include "at91/utility/trace.h"
#include "at91/peripherals/cp15/cp15.h"

#define RSTC_KEY_PASSWORD       (0xA5 << 24)

typedef uint volatile *volatile vmemptr;
#define VMEM(x) (*(vmemptr)(x))
#define VALID_SDRAM_ADDR(x) (((x) >= BOARD_SDRAM_BASE_ADDRESS && (x) < (unsigned int)_sstack))
#define VALID_SDRAM_PTR(x) (VALID_SDRAM_ADDR(x) && (((x) & 0x3) == 0))

//Reference to start of stack defined in linker script
extern unsigned char * _sstack;

/*!
 * @brief Disables the Instruction and Data caches of the ARM926EJ-S core.
 */
static void disableCaches()
{
	CP15_DisableMMU();
	CP15_Disable_D_Cache();
	CP15_Disable_I_Cache();
}

void restart()
{
	TRACE_WARNING_WP("\n\r\t\t ____RESTARTING____\n\r");
	disableCaches();
	gracefulReset();
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

/*
 * Stack backtracing assuming AAPCS and GCC style frame pointer
 * In release: Requires building with -fno-omit-frame-pointer
 * This may not work well with other, non-GCC compilers
 * For LLVM: Will not work since the frame pointer is placed differently
 * This is compatible with AAPCS on AArch64 where the frame pointer is defined
 */
static void print_stack_trace(unsigned int fp)
{
	int prevLr=BOARD_SDRAM_BASE_ADDRESS, lr = 0, depth = 0;

	//Ensure the frame pointer is within valid memory
	if (!VALID_SDRAM_PTR(fp))
	{
		STATIC_PRINT("fp is not a valid pointer: ");
		print_hex(fp);
		STATIC_PRINT("\n\r");
		return;
	}

	do
	{
		prevLr = lr;
		//Adjusting the LR by 4 bytes since LR is the address of the next
		// instruction to be executed after returning back
		lr = VMEM(fp)-4;
		STATIC_PRINT("Backtrace ");
		print_hex(depth);
		STATIC_PRINT(" @ ");
		print_hex(lr);
		STATIC_PRINT("\n\r");
		if (VALID_SDRAM_PTR(fp - 4))
		{
			fp = VMEM(fp - 4);
		}
		else
		{
			break;
		}
	}
	while (lr != 0
			//If the previous LR isn't a lower value than the new one,
			// we're not in a valid, descending stack
			&& prevLr < lr
			//The frame pointer/next LR has to be within valid space
			&& VALID_SDRAM_PTR(fp)
			//Safe guard to avoid infinite loops
			&& ++depth < 20);
}

/*
 * Based on https://wbk.one/article/4/debugging-arm-without-a-debugger-2-abort-handlers
 */
void __attribute__((weak)) handleAbort(unsigned int cpsr, unsigned int spsr,
		unsigned int spCaller, unsigned int lr,
		unsigned int r0, unsigned int r1, unsigned int r2, unsigned int r3,
		unsigned int r4, unsigned int r5, unsigned int r6, unsigned int r7,
		unsigned int r8, unsigned int r9, unsigned int r10, unsigned int r11,
		unsigned int r12, unsigned int lastLR,
		unsigned int dataabort)
{

    //Dump all registers to screen
    STATIC_PRINT("Register dump at crash:\n\r");
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
    STATIC_PRINT("\tr13/sp: "); print_hex(spCaller);STATIC_PRINT("\n\r");
    STATIC_PRINT("\tr14/lr: "); print_hex(lr);      STATIC_PRINT("\n\r");
    STATIC_PRINT("\tcpsr:   "); print_hex(cpsr);    STATIC_PRINT("\n\r");
    STATIC_PRINT("\tspsr:   "); print_hex(spsr);    STATIC_PRINT("\n\r\n\r");

    //Prevent backtracing if in abort mode as that will be a likely crash
	// within this function
	spsr &= 0x1F;
	//23=abort mode
	if (spsr != 23)
	{
		//TODO: Ideally we'd like to know if we have a build with fp here
		print_stack_trace(r11);
	}
	else
	{
		STATIC_PRINT("Abort within abort mode detected. Skipping print stack trace\n\r");
	}

	if (VALID_SDRAM_PTR(lr))
	{
		//This doesn't make too much sense, but for some reason we want to print LR
		// LR is where we would return to if we wanted to continue executing
	    // For most crashes this is not the actual crash location
		STATIC_PRINT("Instruction ");
		print_hex(VMEM(lr));
		STATIC_PRINT(" at ");
		print_hex(lr);
		STATIC_PRINT(" might be at fault (this is the banked lr; actual fault instruction depends on crash type)\n\r");
	}
	else
	{
		STATIC_PRINT("Invalid lr ");
		print_hex(lr);
		STATIC_PRINT(" (not within SDRAM)\n\r");
	}

	//This might be useless information, but printing out lr from
    // the executing mode. In some corner cases it might be useful to know
    // what was the last executed function before crashing.
    STATIC_PRINT("Last call before crash was at ");
    print_hex(lastLR-4);
    STATIC_PRINT(" (might be completely irrelevant info.)\n\r");

    // Read fault codes from the CP15 (coprocessor), page 60 datasheet sam9
    volatile unsigned int faulttype;
    __asm volatile ("mrc p15, 0, %[ft], c5, c0, 0\n\r\t" : [ft] "=r" (faulttype));
    faulttype &= 0xf;

    volatile unsigned int faultdomain;
    __asm volatile ("mrc p15, 0, %[fd], c5, c0, 0\n\r\t" : [fd] "=r" (faultdomain));
    faultdomain >>= 4;
    faultdomain &= 0xf;

    volatile unsigned int datafaultaddr;
    __asm volatile ("mrc p15, 0, %[dfa], c6, c0, 0\n\r\t" : [dfa] "=r" (datafaultaddr));

    STATIC_PRINT("Detected fault type:\n\r");

    // Per fault status register (section 3.3.1, page 3-20, ARM926EJ-S Technical Reference Manual)
    if ((faulttype >> 2) == 0)
    {
    	STATIC_PRINT("\talignment");
    }
    else if (faulttype == 0x5)
    {
    	STATIC_PRINT("\ttranslation");
    }
    else if (faulttype == 0x8)
    {
        STATIC_PRINT("\texternal abort on noncacheable");
    }
    else if (faulttype == 0x9)
    {
    	STATIC_PRINT("\tdomain");
    }
    else if (faulttype == 0xD)
    {
    	STATIC_PRINT("\tpermission");
    }
    else
    {
    	STATIC_PRINT("\tunknown");
    }
    STATIC_PRINT(" (");
    print_hex(faulttype);
    STATIC_PRINT(")\n\r");

    STATIC_PRINT("\tfault domain: ");
    print_hex(faultdomain);
    STATIC_PRINT("\n\r");

    STATIC_PRINT("\tdata fault addr.: ");
    print_hex(datafaultaddr);
    STATIC_PRINT("\n\r");

    cpsr &= 0x1f;
    STATIC_PRINT("cpsr: ");
    print_hex(cpsr);
    STATIC_PRINT(" interpretation:\n\r\t");

    switch(cpsr)
    {
        case 19:
            //This really shouldn't be printed out. How can current mode be supervisor?
        	STATIC_PRINT("Supervisor mode (This is a very unexpected print out)\n\r");
        	break;

        case 23:
            //Abort mode. dataabort should flag whether or not this is prefetch or data abort
        	if (VALID_SDRAM_PTR(lr))
        	{
        		unsigned int tmpLR = lr;
				if (dataabort==0)
				{
					STATIC_PRINT("prefetch abort mode caused by lr - 4 (instruction ");
					tmpLR = lr - 4;
				}
				else
				{
					STATIC_PRINT("data abort mode caused by lr - 8 (instruction ");
					tmpLR = lr - 8;
				}
				print_hex(VMEM(tmpLR));
				STATIC_PRINT(" at ");
				print_hex(tmpLR);
				STATIC_PRINT(")\n\r");
        	}
        	else
        	{
        		STATIC_PRINT("Abort with invalid lr ");
        		print_hex(lr);
        		STATIC_PRINT(" (data abort=");
        		print_hex(dataabort);
        		STATIC_PRINT(")\n\r");
        	}
            break;

        case 27:
            //Undefined instruction mode
            if (VALID_SDRAM_PTR(lr))
        	{
                STATIC_PRINT("undefined instruction caused by ");
                print_hex(VMEM(lr));
                STATIC_PRINT(" at ");
                print_hex(lr);
                STATIC_PRINT("\n\r");
        	}
            else
            {
                STATIC_PRINT("undefined instruction caused at invalid SDRAM address\n\r");
            }
        	break;

        default:
            //Unknown as in FIQ, IRQ, user, system or undefined
            // (hyper and monitor mode should be a thing on this chip)
        	STATIC_PRINT("unknown mode; Please check ARM926EJ-S Technical Reference Manual\n\r");\
        	break;
    }

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

void gracefulReset()
{
	//Manually implementing the TRACE_DEBUG macro to avoid printf
#if (TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
	STATIC_PRINT("-D- gracefulReset - \n\r\t\t gracefulReset\n\r\n\r");
#endif
	// Disable SDRAM else it may cause interference on the EBI
	AT91C_BASE_SDRAMC->SDRAMC_TR = 1;
	AT91C_BASE_SDRAMC->SDRAMC_LPR = AT91C_SDRAMC_LPCB_POWER_DOWN;
	// Reset the peripherals and the processor
	AT91C_BASE_RSTC->RSTC_RCR = AT91C_RSTC_PROCRST | AT91C_RSTC_PERRST | RSTC_KEY_PASSWORD;
}
