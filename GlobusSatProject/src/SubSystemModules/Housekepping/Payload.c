#include "Payload.h"
Boolean state_changed = FALSE;

int payloadRead(unsigned char* buffer, int size, int delay)
{
	for(int i = 0; i < EXTRA_TRIES; i++)
	{
		if(!logError(I2C_write(PAYLOAD_I2C_ADDRESS, GET_LAST_DATA ,1), "I2C write to payload")){return -1;}

		if(!logError(I2C_read(PAYLOAD_I2C_ADDRESS, buffer, size), "I2C read from payload")){return -1;}

		if(buffer[3] == 0)
		{
			return 0;
		}

		vTaskDelay(delay);
	}

	return PAYLOAD_TIMEOUT;
}

int payloadSendCommand(char opcode, unsigned char* buffer, int size, int delay)
{
	if(!logError(I2C_write(PAYLOAD_I2C_ADDRESS, &opcode, 1), "I2C write to payload")){return -1;}

	if(delay > 0)
	{
		vTaskDelay(delay);
	}

	return payloadRead(buffer, size, (delay < 100) ? 5 : delay / 10);
}

void get_radfet_data(radfet_data* radfet)
{
	if (radfet == NULL)
	{
		return;
	}

	char buffer_rad[12];
	char buffer_tmp[8];
	int temp_adc;

	//RADFET
	time_unix curr_time1 = 0;
	Time_getUnixEpoch(&curr_time1);
	radfet->radfet_time = curr_time1;

	if(!logError(payloadSendCommand(READ_RADFET_VOLTAGES, buffer_rad, sizeof(buffer_rad), RADFET_CALC_TIME), "Payload send cmd - radfet")){return;}

	memcpy(&radfet->radfet1, buffer_rad + 4, 4);
	memcpy(&radfet->radfet2, buffer_rad + 8, 4);
	radfet->radfet1 = changeIntIndian(radfet->radfet1);
	radfet->radfet2 = changeIntIndian(radfet->radfet2);

	//TEMPERTURE
	time_unix curr_time2 = 0;
	Time_getUnixEpoch(&curr_time2);
	radfet->temp_time = curr_time2;

	if(!logError(payloadSendCommand(READ_RADFET_TEMP, buffer_tmp, sizeof(buffer_tmp), RADFET_TMP_CALC_TIME), "Payload send cmd - radfet temp")){return;}


	memcpy(&temp_adc, buffer_tmp + 4, 4);
	temp_adc = changeIntIndian(temp_adc);

	// Extract and process ADC value
	int remove_extra_bits = (temp_adc & (~(1 << 29))) >> 5; // Mask and shift to remove redundant bits
	double voltage = ADC_TO_VOLTAGE(remove_extra_bits); // Convert ADC value to voltage
	double temperature = VOLTAGE_TO_TEMPERATURE(voltage); // Convert voltage to temperature

	radfet->temperature = temperature;
}

void get_sel_data(pic32_sel_data* sel)
{
	if (sel == NULL)
	{
		return;
	}

	char buffer[12];
	int* latchups;

	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);
	sel->time = curr_time;

	if(!logError(payloadSendCommand(READ_PIC32_RESETS, buffer, sizeof(buffer), SEL_CALC_TIME), "Payload send cmd - sel")){return;}


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

	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);
	seu->time = curr_time;

	if(!logError(payloadSendCommand(READ_PIC32_UPSETS, buffer, sizeof(buffer), SEU_CALC_TIME), "Payload send cmd - sel")){return;}


	memcpy(&seu->bitFlips_count, buffer + 4, 4);
	seu->bitFlips_count = changeIntIndian(seu->bitFlips_count);

}

int changeIntIndian(int num)
{
	return	((num>>24)&0xff)	|	// move byte 3 to byte 0
			((num<<8)&0xff0000)	|	// move byte 1 to byte 2
			((num>>8)&0xff00)	|	// move byte 2 to byte 1
			((num<<24)&0xff000000);	// byte 0 to byte 3
}
