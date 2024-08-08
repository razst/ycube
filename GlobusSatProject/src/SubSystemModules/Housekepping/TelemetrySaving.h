#include <stdio.h>
#include "utils.h"
#include "TelemetryFiles.h"
#include "TelemetryCollector.h"


typedef struct logDataInRam
{
	unsigned long date;
	logData_t logData;
} logDataInRam;

typedef struct wodDataInRam
{
	unsigned long date;
	WOD_Telemetry_t wodData;
} wodDataInRam;

#define SIZE 10

logDataInRam logArr[SIZE];
int logIndex = 0;

wodDataInRam wodArr[SIZE];
int wodIndex = 0;

int resetArrs();

int saveTlmToRam(void* data, int length, tlm_type_t type);

int getTlm(void* address, int count, tlm_type_t type);
