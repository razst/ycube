#ifndef RAMTLM_H
#define RAMTLM_H

#include <stdio.h>
#include "utils.h"
#include "TelemetryFiles.h"
#include "TelemetryCollector.h"
#include <hal/errors.h>

#define TLM_RAM_SIZE 10000

// struct for saving log TLM data in RAM
typedef struct logDataInRam
{
	time_unix date;
	logData_t logData;
} logDataInRam;

// struct for saving WOD TLM data in RAM
typedef struct wodDataInRam
{
	time_unix date;
	WOD_Telemetry_t wodData;
} wodDataInRam;

// struct for tlm range
typedef struct dataRange
{
	time_unix min;
	time_unix max;
} dataRange;




/*!
 * return range of tlm saved
 * @param type he type of TLM. @see tlm_type_t enum
 * @return struct of range
 */
dataRange getRange(tlm_type_t type);

/*!
 * set all TLM date to 0.
 * @return Error code
 */
int ResetRamTlm();


/*!
 * saves a packet of data in ram.
 * @param data pointer to the data we wish to save.
 * @param length length in bytes of the data.
 * @param type the type of TLM. @see tlm_type_t enum
 * @return Error code
 */
int saveTlmToRam(void* data, int length, tlm_type_t type);

/*!
 * extract packets of data from ram.
 * @param address pointer to the data we wish to save.
 * @param count the number of packets we wish to get.
 * @param type the type of TLM. @see tlm_type_t enum
 * @return number of packets found
 */
int getTlm(void* address, int count, tlm_type_t type);

#endif
