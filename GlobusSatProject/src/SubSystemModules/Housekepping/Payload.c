#include "Payload.h"
Boolean state_changed = FALSE;

PayloadResult payloadRead(unsigned char* buffer, int size, int delay)
{
	int err = 0;

	for(int i = 0; i < EXTRA_TRIES; i++)
	{
		err = I2C_write(PAYLOAD_I2C_ADDRESS, GET_LAST_DATA ,1);
		if(err != E_NO_SS_ERR)
		{
			return PAYLOAD_I2C_Write_Error;
		}

		err = I2C_read(PAYLOAD_I2C_ADDRESS, buffer, size);
		if(err != E_NO_SS_ERR)
		{
			return PAYLOAD_I2C_Read_Error;
		}

		if(buffer[3] == 0)
		{
			return PAYLOAD_SUCCESS;
		}

		vTaskDelay(delay);
	}

	return PAYLOAD_TIMEOUT;
}

PayloadResult payloadSendCommand(char opcode, unsigned char* buffer, int size, int delay)
{

	int err = I2C_write(PAYLOAD_I2C_ADDRESS, &opcode, 1);
	if(err != E_NO_SS_ERR)
	{
		return PAYLOAD_I2C_Write_Error;
	}
	if(delay > 0)
	{
		vTaskDelay(delay);
	}

	return payloadRead(buffer, size, (delay < 100) ? 5 : delay / 10);
	//maybe check type recieved here
}

void get_radfet_data(radfet_data* radfet)
{
	if (radfet == NULL)
	{
		return;
	}

	char buffer_rad[12];
	char buffer_tmp[8];
	PayloadResult res;

	//RADFET
	time_unix curr_time1 = 0;
	Time_getUnixEpoch(&curr_time1);
	radfet->radfet_time = curr_time1;

	res = payloadSendCommand(READ_RADFET_VOLTAGES, buffer_rad, sizeof(buffer_rad), RADFET_CALC_TIME);

	//TEMPERTURE
	time_unix curr_time2 = 0;
	Time_getUnixEpoch(&curr_time2);
	radfet->temp_time = curr_time2;

	res = payloadSendCommand(READ_RADFET_TEMP, buffer_tmp, sizeof(buffer_tmp), RADFET_TMP_CALC_TIME);


}
void get_sel_data(pic32_sel_data* sel)
{
	if (sel == NULL)
	{
		return;
	}

	char buffer[12];
	PayloadResult res;
	int* latchups;

	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);

	res = payloadSendCommand(READ_PIC32_RESETS, buffer, sizeof(buffer), SEL_CALC_TIME);
	sel->time = curr_time;

	if(res != PAYLOAD_SUCCESS)
	{		//??TODO
			return;
	}

	memcpy(latchups, buffer+4, 4);
	if(*latchups == 0) //backup
	{
		memcpy(latchups, buffer+8, 4);
	}
	*latchups = changeIntIndian(*latchups);
	sel->latchUp_count = *latchups;

	FRAM_read((unsigned char*)&sel->sat_reset_count,
	NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);

	FRAM_read((unsigned char*)&sel->eps_reset_count,
	NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);

	sel->battery_state_changed = state_changed;
	state_changed = FALSE;

}
void get_seu_data(pic32_seu_data* seu)
{
	if (seu == NULL)
	{
		return;
	}

	char buffer[8];
	PayloadResult res;
	int* upsets;

	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);

	res = payloadSendCommand(READ_PIC32_UPSETS, buffer, sizeof(buffer), SEU_CALC_TIME);
	seu->time = curr_time;

	if(res != PAYLOAD_SUCCESS)
	{		//??TODO
			return;
	}
	memcpy(upsets, buffer+4, 4);
	*upsets = changeIntIndian(*upsets);
	seu->bitFlips_count = *upsets;

}

int changeIntIndian(int num) //TODO make sure we actually need this
{
	return	((num>>24)&0xff)	|	// move byte 3 to byte 0
			((num<<8)&0xff0000)	|	// move byte 1 to byte 2
			((num>>8)&0xff00)	|	// move byte 2 to byte 1
			((num<<24)&0xff000000);	// byte 0 to byte 3
}
