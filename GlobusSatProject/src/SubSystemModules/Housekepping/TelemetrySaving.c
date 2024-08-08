#include "TelemetrySaving.h"

int zeroingArrs()
{
	for(int i = 0; i < SIZE; i++)
	{
		logArr[i].error = 999;
		//zeroing the other arrays
	}
}

int move(int num) //advance the index acordding to the arr size
{
	num++;
	if (num == SIZE)
	{
		num = 0;
	}
	return num;
}

int saveTlmToRam(void* data, int length, tlm_type_t type)
{
	switch (type)
	{
	case tlm_log:
		memcpy(&logArr[logIndex], data, length);
		logIndex = move(logIndex);

		break;
	default:
		return E_TYPE_ERROR;
	}
	return E_NO_SS_ERR;
}


int getTlm(tlm_type_t type, int count, void* address)
{
	switch (type)
		{
		case tlm_log:
			int index = logIndex;
			for(int i = 0; i < count; i++)
			{
				if(1)
				{
					memcpy(address + i*sizeof(tlm_log), &logArr[index], sizeof(tlm_log));
				}
				index = move(index);
			}

			break;
		default:
			return E_TYPE_ERROR;
		}
		return E_NO_SS_ERR;
}
