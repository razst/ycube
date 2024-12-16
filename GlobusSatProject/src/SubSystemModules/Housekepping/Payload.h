/*
 * Payload.h
 *
 *  Created on: 15 бреб 2024
 *      Author: Magshimim
 */

#ifndef PAYLOAD_H_
#define PAYLOAD_H_

#include <hal/Drivers/I2C.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <hal/Timing/Time.h>
#include "hal/Drivers/I2C.h"
#include <hal/Utility/util.h>
#include "GlobalStandards.h"
#include "utils.h"

#define PAYLOAD_I2C_ADDRESS 0x55

//OPcodes
#define CLEAR_WDT 				0x3F
#define SOFT_RESET 				0xF8
#define READ_PIC32_RESETS 		0x66
#define READ_PIC32_UPSETS 		0x47
#define READ_RADFET_VOLTAGES 	0x33 // ADC state
#define READ_RADFET_TEMP 		0x77 // ADC state
#define DEBUGGING				0x32
#define GET_LAST_DATA			0x45 // in tau-1

//calculation time in payload (ms)
#define RADFET_CALC_TIME		2000	//extra - 200
#define RADFET_TMP_CALC_TIME	1000	//extra - 100
#define SEU_CALC_TIME			200		//extra - 20
#define SEL_CALC_TIME			20		//extra - 5

#define EXTRA_TRIES		20

#define ADC_TO_VOLTAGE(R) ((2 * 4.096 * (R)) / (2 << 23))
#define VOLTAGE_TO_TEMPERATURE(V) (100 * ((V) * (5 / 2.0) - 2.73))

extern Boolean state_changed;

typedef struct __attribute__ ((__packed__)) radfet_data
{
	time_unix radfet_time;

	int radfet1;		/**< ADC conversion result for RADFET 1 */
	int radfet2;		/**< ADC conversion result for RADFET 2 */

	time_unix temp_time;

	double temperature;				/**< Temperature measurement in degrees Celsius */

} radfet_data;

typedef struct __attribute__ ((__packed__)) pic32_sel_data
{
	time_unix time;
	unsigned int latchUp_count;
	unsigned int sat_reset_count;
	unsigned int eps_reset_count;
	Boolean battery_state_changed;

} pic32_sel_data;

typedef struct __attribute__ ((__packed__)) pic32_seu_data
{
	time_unix time;
	unsigned int bitFlips_count;

} pic32_seu_data;

/*!
 * read the data of teh last task
 * @param buffer to restore the data, size of recieved data, delay time(ms) if the data isn't ready
 * @return result status
 */
int payloadRead(unsigned char* buffer, int size, int delay);

/*!
 * send command to be executed, then call payloadRead to get results
 * @param opcode to the executed command, buffer to restore the data, size of recieved data, delay time(ms) to wait for response
 * @return result status
 */
int payloadSendCommand(char opcode, unsigned char* buffer, int size, int delay);

/*!
 * execute radfet commands
 * @param[out] output radfet_data
 */
void get_radfet_data(radfet_data* radfet);

/*!
 * execute sel commands
 * @param[out] output pic32_sel_data
 */
void get_sel_data(pic32_sel_data* sel);

/*!
 * execute seu commands
 * @param[out] output pic32_seu_data
 */
void get_seu_data(pic32_seu_data* seu);


/*!
 * swap byte order
 * @param[in] number to convert(int)
 * @return converted number(int)
 */
int changeIntIndian(int num);
#endif /* PAYLOAD_H_ */
