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
	//RADFET
	time_unix curr_time1 = 0;
	Time_getUnixEpoch(&curr_time1);


	//TEMPERTURE
	time_unix curr_time2 = 0;
	Time_getUnixEpoch(&curr_time2);

}
void get_sel_data(pic32_sel_data* sel)
{
	if (sel == NULL)
	{
		return;
	}
	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);


	pic32_sel_data data = {0};

	FRAM_read((unsigned char*)&data.sat_reset_count,
	NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);

	FRAM_read((unsigned char*)&data.eps_reset_count,
	NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);

	data.battery_state_changed = state_changed;
	state_changed = FALSE;

}
void get_seu_data(pic32_seu_data* seu)
{
	if (seu == NULL)
	{
		return;
	}

	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);

}
