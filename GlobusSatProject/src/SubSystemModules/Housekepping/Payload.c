/*
 * Payload.c
 *
 *  Created on: 15 бреб 2024
 *      Author: Magshimim
 */
#include "Payload.h"

SoreqResult payloadRead(int size,unsigned char* buffer) // replace size, buffer
{
	unsigned char wtd_and_read[] = {GET_LAST_DATA}; // do we need array??
	int err=0;

	int i=0;
	do
	{
		err = I2C_write(PAYLOAD_I2C_ADDRESS, wtd_and_read ,1);
		if(err != 0)// ERR_
		{
			return PAYLOAD_I2C_Write_Error;
		}
		vTaskDelay(READ_DELAY);
		err = I2C_read(SOREQ_I2C_ADDRESS, buffer, size);
		if(err != 0)
		{
			return PAYLOAD_I2C_Read_Error;
		}
		if(buffer[3] == 0)
		{
			return PAYLOAD_SUCCESS;
		}
	} while(TIMEOUT/READ_DELAY > i++);
	return PAYLOAD_TIMEOUT;
}

SoreqResult payloadSendCommand(char opcode, int size, unsigned char* buffer,int delay) // buffer before size
{
	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);

	int err = I2C_write(PAYLOAD_I2C_ADDRESS, &opcode, 1);
	if(err != 0) // ERR_
	{
		return PAYLOAD_I2C_Write_Error;
	}
	if(delay > 0)
	{
		vTaskDelay(delay);
	}
	SoreqResult res =  payloadRead(size, buffer);

	return res;
}

void get_radfet_data(radfet_data* radfet)
{
	if (radfet == NULL)
	{
		return;
	}
}
void get_sel_data(pic32_sel_data* sel)
{
	if (sel == NULL)
	{
		return;
	}
}
void get_seu_data(pic32_seu_data* seu)
{
	if (seu == NULL)
	{
		return;
	}
}
