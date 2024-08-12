#ifndef RAMTLM_H
#define RAMTLM_H

#include <stdio.h>
#include "utils.h"
#include "TelemetryFiles.h"
#include "TelemetryCollector.h"
#include <hal/errors.h>

#define TLM_RAM_SIZE 10

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



logDataInRam logArr[TLM_RAM_SIZE];
int logIndex = 0;

wodDataInRam wodArr[TLM_RAM_SIZE];
int wodIndex = 0;


/*!
 * set all TLM date to 0.
 * @return Error code
 */
int resetArrs();

/*!
 * set all TLM date to 0.
 * @param data Address where data to be written is stored.
 * @param length length in bytes of the data.
 * @param type the type of TLM. @see tlm_type_t enum
 * @return Error code
 */
int saveTlmToRam(void* data, int length, tlm_type_t type);

/*
 *
 */
int getTlm(void* address, int count, tlm_type_t type);

#endif
