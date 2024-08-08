#include "TelemetrySaving.h"
#include <hal/errors.h>

int resetArrs() {
	for (int i = 0; i < SIZE; i++) {
		logArr[i].date = 0;
		wodArr[i].date = 0;
		//zeroing the other arrays
	}
}

int move(int num) //advance the index acordding to the arr size
{
	num++;
	if (num == SIZE) {
		num = 0;
	}
	return num;
}

int saveTlmToRam(void* data, int length, tlm_type_t type) {
	switch (type) {
	case tlm_log:
		Time_getUnixEpoch(&logArr[logIndex].date);
		memcpy(&logArr[logIndex].logData, data, length);
		logIndex = move(logIndex);
		break;

	case tlm_wod:
		Time_getUnixEpoch(&wodArr[logIndex].date);
		memcpy(&wodArr[wodIndex].wodData, data, length);
		wodIndex = move(wodIndex);
		break;

	default:
		return E_TYPE_ERROR;
	}
	return E_NO_SS_ERR;
}

int getTlm(void* address, int count, tlm_type_t type) {
	int filledCount = 0;
	int index;
	switch (type) {
	case tlm_log:
		index = logIndex;
		for (int i = 0; i < count; i++)
		{
			if (logArr[index].date != 0)
			{
				memcpy(address + i * sizeof(logDataInRam), &logArr[index], sizeof(logDataInRam));
				filledCount++;
			}
			index = move(index);
		}
		break;

	case tlm_wod:
		index = wodIndex;
		for (int i = 0; i < count; i++)
		{
			if (wodArr[index].date != 0)
			{
				memcpy(address + i * sizeof(wodDataInRam), &wodArr[index], sizeof(wodDataInRam));
				filledCount++;
			}
			index = move(index);
		}
		break;

	default:
		return E_TYPE_ERROR;
	}
	return E_NO_SS_ERR;
}
