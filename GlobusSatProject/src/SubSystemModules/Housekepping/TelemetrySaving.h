#include <stdio.h>
#include "utils.h"
#include "errors.h" //error?
#include "TelemetryFiles.h"

#define SIZE 10

logData_t logArr[SIZE];
int logIndex = 0;

int zeroingArrs();

int saveTlmToRam(void* data, int length, tlm_type_t type);

int getTlm(tlm_type_t type, int count, void* address);
