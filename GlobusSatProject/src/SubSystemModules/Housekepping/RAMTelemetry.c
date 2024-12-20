#include "RAMTelemetry.h"


//array to save log data
logDataInRam logArr[TLM_RAM_SIZE];
//index for the current saving place in array
int logIndex = 0;

//array to save wod data
wodDataInRam wodArr[TLM_RAM_SIZE];
//index for the current saving place in array
int wodIndex = 0;

//array to save radfet data
radfetDataInRam radfetArr[TLM_RAM_SIZE];
//index for the current saving place in array
int radfetIndex = 0;

//array to save sel data
selDataInRam selArr[TLM_RAM_SIZE];
//index for the current saving place in array
int selIndex = 0;

//array to save seu data
seuDataInRam seuArr[TLM_RAM_SIZE];
//index for the current saving place in array
int seuIndex = 0;

int ResetRamTlm() {
	for (int i = 0; i < TLM_RAM_SIZE; i++) {
		logArr[i].date = 0;
		wodArr[i].date = 0;
		radfetArr[i].date = 0;
		selArr[i].date = 0;
		seuArr[i].date = 0;
		//zeroing the other arrays
	}
	return 0;
}

int inc(int num) //advance the index acordding to the arr size
{
	num++;
	if (num == TLM_RAM_SIZE) {
		num = 0;
	}
	return num;
}


int dec(int num) //advance the index acordding to the arr size
{
	num--;
	if (num < 0) {
		num = TLM_RAM_SIZE-1;
	}
	return num;
}


int saveTlmToRam(void* data, int length, tlm_type_t type) {
	switch (type)
	{
	case tlm_log:
		Time_getUnixEpoch(&logArr[logIndex].date);
		memcpy(&logArr[logIndex].logData, data, length);
		logIndex = inc(logIndex);
		break;

	case tlm_wod:
		Time_getUnixEpoch(&wodArr[wodIndex].date);
		memcpy(&wodArr[wodIndex].wodData, data, length);
		wodIndex = inc(wodIndex);
		break;

	case tlm_radfet:
		Time_getUnixEpoch(&radfetArr[radfetIndex].date);
		memcpy(&radfetArr[radfetIndex].radfetData, data, length);
		radfetIndex = inc(radfetIndex);
		break;

	case tlm_sel:
		Time_getUnixEpoch(&selArr[selIndex].date);
		memcpy(&selArr[selIndex].selData, data, length);
		selIndex = inc(selIndex);
		break;

	case tlm_seu:
		Time_getUnixEpoch(&seuArr[seuIndex].date);
		memcpy(&seuArr[seuIndex].seuData, data, length);
		seuIndex = inc(seuIndex);
		break;

	default:
		return INVALID_TLM_TYPE;
	}
	return E_NO_SS_ERR;
}

int getTlm(void* address, int count, tlm_type_t type)
{
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

	case tlm_radfet:
		index = radfetIndex;
		arr = radfetArr;
		length = sizeof(radfetDataInRam);
		break;

	case tlm_sel:
		index = selIndex;
		arr = selArr;
		length = sizeof(selDataInRam);
		break;

	case tlm_seu:
		index = seuIndex;
		arr = seuArr;
		length = sizeof(seuDataInRam);
		break;

	default:
		return -1;
	}

	for (int i = 0; i < TLM_RAM_SIZE && filledCount < count; i++)
	{
		index = dec(index);
		memcpy(&time, arr + index * length, sizeof(time_unix));
		if (time != 0)
		{
			memcpy(address + filledCount * length, arr + index * length, length);
			filledCount++;
		}
	}
	return filledCount;
}

dataRange getRange(tlm_type_t type)
{
	dataRange range;
	Boolean flag = FALSE;

	int index;
	void* arr;
	int typeLength;
	time_unix time;

	switch (type)
	{
	case tlm_log:
		index = logIndex;
		arr = logArr;
		typeLength = sizeof(logDataInRam);
		break;

	case tlm_wod:
		index = wodIndex;
		arr = wodArr;
		typeLength = sizeof(wodDataInRam);
		break;

	case tlm_radfet:
		index = radfetIndex;
		arr = radfetArr;
		typeLength = sizeof(radfetDataInRam);
		break;

	case tlm_sel:
		index = selIndex;
		arr = selArr;
		typeLength = sizeof(selDataInRam);
		break;

	case tlm_seu:
		index = seuIndex;
		arr = seuArr;
		typeLength = sizeof(seuDataInRam);
		break;
	}

	range.min = 0;
	range.max = 0;


	for (int i = 0; i < TLM_RAM_SIZE; i++)
	{
		time_unix date;
		memcpy(&date, arr + i*typeLength, sizeof(time_unix));
		if((date < range.min || range.min == 0) && date != 0)
		{
			range.min = date;
		}
		if(date > range.max && date != 0)
		{
			range.max = date;
		}
	}
	return range;
}

