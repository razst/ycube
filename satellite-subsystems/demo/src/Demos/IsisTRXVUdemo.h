/*
 * IsisTRXVUdemo.h
 *
 *  Created on: 6 feb. 2015
 *      Author: malv
 */

#ifndef ISISTRXVUDEMO_H_
#define ISISTRXVUDEMO_H_

#include <hal/boolean.h>

#define I2C_TRXVU_TC_ADDR 0x61		//!< I2C address of the transceiver


typedef struct __attribute__ ((__packed__))
{
	int delay;
	int amount;
} beacon_arguments_t;

typedef struct __attribute__ ((__packed__)) WOD_Telemetry_t
{
	unsigned short vbat;					///< the current voltage on the battery [mV]
	unsigned short volt_5V;				///< the current voltage on the 5V bus [mV]
	unsigned short volt_3V3;				///< the current voltage on the 3V3 bus [mV]
	unsigned short charging_power;			///< the current charging power [mW]
	unsigned short consumed_power;			///< the power consumed by the satellite [mW]
	unsigned short electric_current;		///< the up-to-date electric current of the battery [mA]
	unsigned short current_3V3;			///< the up-to-date 3.3 Volt bus current of the battery [mA]
	unsigned short current_5V;			///< the up-to-date 5 Volt bus current of the battery [mA]
	unsigned short mcu_temp; 				/*!< Measured temperature provided by a sensor internal to the MCU in raw form */
	unsigned short bat_temp; 				/*!< 2 cell battery pack: not used 4 cell battery pack: Battery pack temperature on the front of the battery pack. */
	//int32_t solar_panels[6];                // temp of each solar panel
	unsigned long sat_time;				///< current Unix time of the satellites clock [sec]
	unsigned int free_memory;		///< number of bytes free in the satellites SD [byte]
	unsigned int corrupt_bytes;		///< number of currpted bytes in the memory	[bytes]
	unsigned int number_of_resets;	///< counts the number of resets the satellite has gone through [#]
	unsigned long sat_uptime;			///< Sat uptime
	unsigned int photo_diodes[5]; 			// photo diodes
	unsigned int num_of_cmd_resets;///< counts the number of resets the satellite has gone through due to ground station command [#]
} WOD_Telemetry_t;
/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean IsisTRXVUdemoMain(void);

/***
 * Initializes the TRXVU subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean IsisTRXVUdemoInit(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 * note:
 * Depends on an initialized TRXVU subsystem driver.
 */
void IsisTRXVUdemoLoop(void);

/***
 * (obsolete) Legacy function to start interactive session
 * Always returns TRUE
 *
 * Note:
 * Use IsisTRXVUdemoMain instead.
 */
Boolean TRXVUtest(void);

int createSendBeaconTask();
/***
 *
 */
int sendBeaconTask(void* args);
/***
 *
 */
int createRandBeacon(WOD_Telemetry_t* beacon);

#endif /* ISISTRXVUDEMO_H_ */
