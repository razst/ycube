#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "GlobalStandards.h"

#include <hal/Timing/Time.h>
#include "SubSystemModules/Communication/SatCommandHandler.h"


#include <satellite-subsystems/isis_vu_e.h>
#include <satellite-subsystems/isis_ants.h>


#include <hcc/api_fat.h>
#include <hal/Drivers/I2C.h>
#include <stdlib.h>
#include <string.h>
#include <hal/errors.h>
#include "TLM_management.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "Maintanence_Commands.h"

// data in SPL should be: slaveaddr,size of data to get back from the I2C command ,data to sent to I2C
int CMD_GenericI2C(sat_packet_t *cmd)
{
	if(cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	unsigned int slaveAddr = 0;
	unsigned int length = cmd->data[sizeof(slaveAddr)];
	unsigned char *data = cmd->data;

	memcpy(&slaveAddr, cmd->data, sizeof(slaveAddr));

	err = I2C_write(slaveAddr,data + sizeof(slaveAddr) + sizeof(length), length);

	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}

	return err;
}

// SPL data should be: addr(int),size of data to read
int CMD_FRAM_ReadAndTransmitt(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	unsigned int addr = 0;
	unsigned int size = 0;

	memcpy(&addr, cmd->data, sizeof(addr));
	memcpy(&size, cmd->data + sizeof(addr),sizeof(size));

	int data=0;
	err = FRAM_read((unsigned char*)&data, addr, size);
	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, &data, size);
	}

	return err;
}

// SPL data should be: addr(int),data
int CMD_FRAM_WriteAndTransmitt(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	unsigned int addr = 0;
	unsigned short length = cmd->data[sizeof(addr)];
	unsigned char *data = cmd->data;

	memcpy(&addr, cmd->data, sizeof(addr));

	err = FRAM_write(data + sizeof(addr) + sizeof(length), addr, length);
	if (err != E_NO_SS_ERR){
		return err;
	}
	err = FRAM_read(data, addr, length);
	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, data, length);
	}
	return err;
}

int CMD_FRAM_Start(sat_packet_t *cmd)
{
	int err = 0;
	err = FRAM_start();
	if (err == E_NO_SS_ERR)
	{
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)&err, sizeof(err));
	}
	return err;
}

int CMD_FRAM_Stop(sat_packet_t *cmd)
{

	FRAM_stop();
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	return 0;
}

int CMD_FRAM_ReStart(sat_packet_t *cmd)
{
	FRAM_stop();
	int err = FRAM_start();
	if (err == E_NO_SS_ERR)
	{
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}
	return err;
}

int CMD_FRAM_GetDeviceID(sat_packet_t *cmd)
{

	unsigned char id;
	int err = FRAM_getDeviceID(&id);
	if (err == E_NO_SS_ERR)
	{
		TransmitDataAsSPL_Packet(cmd, &id, sizeof(id));
	}
	return err;
}

int CMD_UpdateSatTime(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	time_unix set_time = 0;
	memcpy(&set_time, cmd->data, sizeof(set_time));
	err = Time_setUnixEpoch(set_time);
	if (err == E_NO_SS_ERR)
	{
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)&set_time, sizeof(set_time));
	}
	ResetGroundCommWDT();// make sure the CommWDT is in sync with the new time
	return err;
}

int CMD_GetSatTime(sat_packet_t *cmd)
{
	int err = 0;
	time_unix curr_time = 0;
	err = Time_getUnixEpoch(&curr_time);
	if (err == E_NO_SS_ERR)
	{
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)&curr_time, sizeof(curr_time));
	}

	return err;
}

int CMD_GetDevInfo(sat_packet_t *cmd)
{
	char message[135] = "Written, developed and tested by: Ishay Dayan, Nave Adany, Uriel Adler, Dror Cohen, David Raviv, Israel Yehuda. Mentor: Raz Steinmetz";
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&message, sizeof(message));
}

int CMD_GetSatUptime(sat_packet_t *cmd)
{

	time_unix uptime = 0;
	uptime = Time_getUptimeSeconds();
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&uptime, sizeof(uptime));
	return 0;
}

int CMD_SoftTRXVU_ComponenetReset(sat_packet_t *cmd)
{
	int err = 0;
	err = isis_vu_e__reset_wdg_rx(ISIS_TRXVU_I2C_BUS_INDEX);
	vTaskDelay(1 / portTICK_RATE_MS);
	err += isis_vu_e__reset_wdg_tx(ISIS_TRXVU_I2C_BUS_INDEX);
	return err;
}

int CMD_HardTRXVU_ComponenetReset(sat_packet_t *cmd)
{
	int err = 0;
	err = isis_vu_e__reset_rx(ISIS_TRXVU_I2C_BUS_INDEX);
	vTaskDelay(1 / portTICK_RATE_MS);
	err += isis_vu_e__reset_tx(ISIS_TRXVU_I2C_BUS_INDEX);
	return err;
}


int CMD_ResetComponent(sat_packet_t *cmd)
{
	int err = 0;

	Boolean8bit reset_flag = TRUE_8BIT;

	char restType=0;
	memcpy(&restType, cmd->data, sizeof(char));

	switch (restType)
	{
	case reset_software:
		SendAckPacket(ACK_SOFT_RESET, cmd, NULL, 0);
		FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);
		vTaskDelay(10);
		restart();
		break;
	case reset_eps: // this is a soft reset of the MCU
		SendAckPacket(ACK_EPS_RESET, cmd, NULL, 0);
		FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);
		vTaskDelay(10);
		HardResetMCU();
		break;

	case reset_trxvu_hard:
		SendAckPacket(ACK_TRXVU_HARD_RESET, cmd, NULL, 0);
		setTransponderEndTime(0);
		logError(isis_vu_e__reset_hw_rx(ISIS_TRXVU_I2C_BUS_INDEX),"CMD_ResetComponent-IsisTrxvu_softReset rx");
		vTaskDelay(1 / portTICK_RATE_MS);
		logError(isis_vu_e__reset_hw_tx(ISIS_TRXVU_I2C_BUS_INDEX),"CMD_ResetComponent-IsisTrxvu_softReset tx");
		vTaskDelay(100);
		break;

	case reset_trxvu_soft:
		SendAckPacket(ACK_TRXVU_SOFT_RESET, cmd, NULL, 0);
		setTransponderEndTime(0);
		logError(isis_vu_e__reset_wdg_rx(ISIS_TRXVU_I2C_BUS_INDEX),"CMD_ResetComponent-IsisTrxvu_softReset rx");
		vTaskDelay(1 / portTICK_RATE_MS);
		logError(isis_vu_e__reset_wdg_tx(ISIS_TRXVU_I2C_BUS_INDEX),"CMD_ResetComponent-IsisTrxvu_softReset tx");
		vTaskDelay(100);
		break;

	case reset_filesystem:

		DeInitializeFS(0);

		vTaskDelay(10);
		err = (unsigned int) InitializeFS(FALSE);
		vTaskDelay(10);
		if (err == E_NO_SS_ERR) SendAckPacket(ACK_FS_RESET, cmd, (unsigned char*) &err, sizeof(err));
		break;

	case reset_ant_SideA:
		err = logError(isis_ants__reset(0),"CMD_ResetComponent-IsisAntS_reset A");
		if (err == E_NO_SS_ERR) SendAckPacket(ACK_ANTS_RESET, cmd, (unsigned char*) &err, sizeof(err));
		break;

	case reset_ant_SideB:
		err = logError(isis_ants__reset(1),"CMD_ResetComponent-IsisAntS_reset B");
		if (err == E_NO_SS_ERR) SendAckPacket(ACK_ANTS_RESET, cmd, (unsigned char*) &err, sizeof(err));
		break;

	default:
		SendAckPacket(ACK_UNKNOWN_SUBTYPE, cmd, NULL, 0);
		break;
	}
	vTaskDelay(10);
	return err;
}
