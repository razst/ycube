#include "RAMTelemetry.h"

logDataInRam logArr[TLM_RAM_SIZE];
int logIndex = 0;

wodDataInRam wodArr[TLM_RAM_SIZE];
int wodIndex = 0;


int resetArrs() {
	for (int i = 0; i < TLM_RAM_SIZE; i++) {
		logArr[i].date = 0;
		wodArr[i].date = 0;
		//zeroing the other arrays
	}
	return 0;
}

int move(int num) //advance the index acordding to the arr size
{
	num++;
	if (num == TLM_RAM_SIZE) {
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
		Time_getUnixEpoch(&wodArr[wodIndex].date);
		memcpy(&wodArr[wodIndex].wodData, data, length);
		wodIndex = move(wodIndex);
		break;

	default:
		return INVALID_TLM_TYPE;
	}
	return E_NO_SS_ERR;
}

int getTlm(void* address, int count, tlm_type_t type) {
	int filledCount = 0;
	int index;
	void* arr;
	int length;
	time_unix time;

	switch (type) {
	case tlm_log:
		index = logIndex;
		arr = logArr;
		length = sizeof(logDataInRam);
		break;

	case tlm_wod:
		index = wodIndex;
		arr = wodArr;
		length = sizeof(wodDataInRam);
		break;

	default:
		return filledCount;
	}

	for (int i = 0; i < TLM_RAM_SIZE && filledCount < count; i++)
	{
		memcpy(&time, &arr[index], sizeof(time_unix));
		if (time != 0)
		{
			memcpy(address + i * length, &arr[index], length);
			filledCount++;
		}
		index = move(index);
	}
	return filledCount;
}
