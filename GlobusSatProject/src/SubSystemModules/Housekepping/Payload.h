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


typedef enum {
    PAYLOAD_SUCCESS, PAYLOAD_I2C_Write_Error, PAYLOAD_I2C_Read_Error, PAYLOAD_TIMEOUT
} PayloadResult;

typedef struct __attribute__ ((__packed__)) radfet_data
{
	time_unix radfet_time;

	//readfet 1&2 readout

	time_unix temp_time;

	//temperture readout
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
PayloadResult payloadRead(unsigned char* buffer, int size, int delay);

/*!
 * send command to be executed, then call payloadRead to get results
 * @param opcode to the executed command, buffer to restore the data, size of recieved data, delay time(ms) to wait for response
 * @return result status
 */
PayloadResult payloadSendCommand(char opcode, unsigned char* buffer, int size, int delay);

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
#endif /* PAYLOAD_H_ */
