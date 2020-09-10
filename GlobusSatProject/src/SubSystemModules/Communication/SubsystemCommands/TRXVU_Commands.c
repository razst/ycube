#include <string.h>
#include <stdlib.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include "GlobalStandards.h"
#include "TRXVU_Commands.h"
#include "TLM_management.h"


extern xTaskHandle xDumpHandle;			                //task handle for dump task
extern xSemaphoreHandle xDumpLock;                      // this global lock is defined once in TRXVU.c

static dump_arguments_t dmp_pckt;

void DumpTask(void *args) {
	if (args == NULL) {
		FinishDump(NULL, NULL, ACK_DUMP_ABORT, NULL, 0);
		return;
	}


	// start the SD FS for this dump task
	logError(f_enterFS(), "DumpTask-f_enterFS");

	//dump_arguments_t t = *((dump_arguments_t*)args);
	dump_arguments_t *task_args = (dump_arguments_t *) args;

	SendAckPacket(ACK_DUMP_START, &task_args->cmd,
			NULL, 0);

	stopDump = FALSE;
	int numOfElementsSent = 0;
	// calculate how many days we were asked to dump (every day has 86400 seconds)
	int numberOfDays = (task_args->t_end - task_args->t_start)/86400;
	if (task_args->cmd.cmd_subtype == DUMP_DAYS){
		Time date;
		timeU2time(task_args->t_start,&date);
		numOfElementsSent = readTLMFiles(task_args->dump_type,date,numberOfDays,task_args->cmd.ID,task_args->resulotion);
	}else{ // DUMP_TIME_RANGE
		numOfElementsSent = readTLMFileTimeRange(task_args->dump_type,task_args->t_start,task_args->t_end,task_args->cmd.ID,task_args->resulotion);
	}

	if (numOfElementsSent<0){
		FinishDump(task_args, NULL, ACK_ERROR_MSG, &numOfElementsSent, sizeof(numOfElementsSent));
	}else{
		FinishDump(task_args, NULL, ACK_DUMP_FINISHED, &numOfElementsSent, sizeof(numOfElementsSent));
	}
	vTaskDelete(NULL); // kill the dump task

}

int CMD_AntennaDeploy(sat_packet_t *cmd)
{

//	while (TRUE)
//	{
//		printf("******* REMARK - ANT DEPLOY - ANT DEPLOY - ANT DEPLOY - ANT DEPLOY\n");
//		vTaskDelay(SECONDS_TO_TICKS(10));
//	}



	int err = logError(IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX , isisants_sideA, isisants_arm) ,"CMD_AntennaDeploy-IsisAntS_setArmStatus-A");
	if (err == E_NO_SS_ERR)
		logError(IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA,ANTENNA_DEPLOYMENT_TIMEOUT) ,"CMD_AntennaDeploy-IsisAntS_autoDeployment-A");

	logError(IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX , isisants_sideB, isisants_arm) ,"CMD_AntennaDeploy-IsisAntS_setArmStatus-B");
	if (err == E_NO_SS_ERR)
		logError(IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB,ANTENNA_DEPLOYMENT_TIMEOUT),"CMD_AntennaDeploy-IsisAntS_autoDeployment-B");


	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
	}

	return err;

}


int CMD_StartDump(sat_packet_t *cmd)
{
	if (NULL == cmd) {
		return -1;
	}



	//dump_arguments_t *dmp_pckt = malloc(sizeof(*dmp_pckt));
	unsigned int offset = 0;

	//dmp_pckt.cmd.ID = cmd->ID;
	// copy all cmd data...
	AssembleCommand(&cmd->data,cmd->length,cmd->cmd_type,cmd->cmd_subtype,cmd->ID, &dmp_pckt.cmd);

	memcpy(&dmp_pckt.dump_type, cmd->data, sizeof(dmp_pckt.dump_type));
	offset += sizeof(dmp_pckt.dump_type);

	memcpy(&dmp_pckt.t_start, cmd->data + offset, sizeof(dmp_pckt.t_start));
	offset += sizeof(dmp_pckt.t_start);

	memcpy(&dmp_pckt.t_end, cmd->data + offset, sizeof(dmp_pckt.t_end));
	offset += sizeof(dmp_pckt.t_end);

	// check for invalid dump parametrs
	if (dmp_pckt.t_start>=dmp_pckt.t_end){
		return E_INVALID_PARAMETERS; // exit with error
	}

	memcpy(&dmp_pckt.resulotion, cmd->data + offset, sizeof(dmp_pckt.resulotion));


	if (xSemaphoreTake(xDumpLock,SECONDS_TO_TICKS(WAIT_TIME_SEM_DUMP)) != pdTRUE) {
		return E_GET_SEMAPHORE_FAILED;
	}
	xTaskCreate(DumpTask, (const signed char* const )"DumpTask", 2000,
			&dmp_pckt, configMAX_PRIORITIES - 2, &xDumpHandle);

	SendAckPacket(ACK_DUMP_START, cmd,NULL,0);

	return 0;
}



int CMD_SendDumpAbortRequest(sat_packet_t *cmd)
{
	SendDumpAbortRequest();
	//stopDump = TRUE;
	SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
	return 0;
}

int CMD_ForceDumpAbort(sat_packet_t *cmd)
{
	AbortDump(cmd);
	SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
	return 0;
}

int CMD_SetTransponder(sat_packet_t *cmd)
{
	//sends I2C command
	int err = 0;
	time_unix duration = 0;
	char data[2] = {0, 0};

	data[0] = 0x38;
	data[1] = cmd->data[0];

	//memcpy(data[1],cmd->data[0],sizeof(char)); //data[1] = 0x02 - transponder or data[1] = 0x01 - nominal


	if(data[1] == trxvu_transponder_on){
		time_unix curr_tick_time = 0;
		Time_getUnixEpoch(&curr_tick_time);
		if (curr_tick_time < getMuteEndTime()) return TRXVU_TRANSPONDER_WHILE_MUTE;
		SetIdleState(trxvu_idle_state_off, 0);
		memcpy(&duration,cmd->data + sizeof(char),sizeof(duration));
		if(duration > MAX_TRANS_TIME) return TRXVU_TRANSPONDER_TOO_LONG;

		turnOnTransponder();
		setTransponderEndTime(curr_tick_time + duration);

	}else if (data[1] == trxvu_transponder_off){
		err = I2C_write(I2C_TRXVU_TC_ADDR, data, 2);
		setTransponderEndTime(0);

	}else {
		return E_INVALID_PARAMETERS;
	}

	if (err == E_NO_SS_ERR)
		SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
	return err;
}


int CMD_SetRSSITransponder(sat_packet_t *cmd)
{
	short rssiValue = 0;
	memcpy(&rssiValue,cmd->data,sizeof(rssiValue));

	int err = SetRSSITransponder(rssiValue);

	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
		// update RSSI value in FRAM so we can use it after reset
		setTransponderRSSIinFRAM(rssiValue);
	}

	return err;
}


int CMD_MuteTRXVU(sat_packet_t *cmd)
{

	// turn off Idle
	SetIdleState(trxvu_idle_state_off, 0);

	// turn off the transponder
	setTransponderEndTime(0);
	char data[2] = {0, 0};
	data[0] = 0x38;
	data[1] = trxvu_transponder_off;
	I2C_write(I2C_TRXVU_TC_ADDR, data, 2);

	int err = 0;
	time_unix mute_duaration = 0;
	memcpy(&mute_duaration,cmd->data,sizeof(mute_duaration));
	err = muteTRXVU(mute_duaration);
	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
	}
	return err;
}

int CMD_SetIdleState(sat_packet_t *cmd)
{
	char state;
	memcpy(&state,cmd->data,sizeof(state));
	time_unix duaration = 0;
	if (state == trxvu_idle_state_on){
		memcpy(&duaration,cmd->data+sizeof(state),sizeof(duaration));
	}

	int err = SetIdleState(state,duaration);

	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}

	return err;
}


int CMD_UnMuteTRXVU(sat_packet_t *cmd)
{
	UnMuteTRXVU();
	SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	return 0;
}

int CMD_GetBaudRate(sat_packet_t *cmd)
{
	//ISIStrxvuBitrateStatus bitrate;
	ISIStrxvuTransmitterState trxvu_state;
	int err = IsisTrxvu_tcGetState(ISIS_TRXVU_I2C_BUS_INDEX, &trxvu_state);

	if (err == E_NO_SS_ERR){
		int bitrate = trxvu_state.fields.transmitter_bitrate; //isn't bitrate a char?
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &bitrate, sizeof(bitrate));
	}

	return err;
}


int CMD_TrasmitBeacon(sat_packet_t *cmd){
	int err = BeaconLogic(TRUE);
	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}
}


int CMD_GetBeaconInterval(sat_packet_t *cmd)
{
	int err = 0;
	time_unix beacon_interval = 0;
	err = FRAM_read((unsigned char*) &beacon_interval,
			BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &beacon_interval,
				sizeof(beacon_interval));
	}
	return err;
}


int CMD_SetBaudRate(sat_packet_t *cmd)
{
	ISIStrxvuBitrateStatus bitrate;
	bitrate = (ISIStrxvuBitrateStatus) cmd->data[0];
	int err = IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, bitrate);
	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}
	return err;
}


int CMD_GetTxUptime(sat_packet_t *cmd)
{
	int err = 0;
	time_unix uptime = 0;
	err = IsisTrxvu_tcGetUptime(ISIS_TRXVU_I2C_BUS_INDEX, (unsigned int*)&uptime);
	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)&uptime, sizeof(uptime));
	}

	return err;
}

int CMD_GetRxUptime(sat_packet_t *cmd)
{
	int err = 0;
	time_unix uptime = 0;
	err = IsisTrxvu_rcGetUptime(ISIS_TRXVU_I2C_BUS_INDEX,(unsigned int*) &uptime);
	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &uptime, sizeof(uptime));
	}

	return err;
}


int CMD_GetNumOfOnlineCommands(sat_packet_t *cmd)
{
	int err = 0;
	unsigned short int temp = 0;
	err = IsisTrxvu_rcGetFrameCount(ISIS_TRXVU_I2C_BUS_INDEX, &temp);
	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &temp, sizeof(temp));
	}

	return err;
}

int CMD_AntSetArmStatus(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL) {
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	ISISantsSide ant_side = cmd->data[0];

	ISISantsArmStatus status = cmd->data[1];
	err = IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, ant_side, status);

	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}

	return err;
}

int CMD_AntGetArmStatus(sat_packet_t *cmd)
{
	int err = 0;
	ISISantsStatus status;
	ISISantsSide ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));

	err = IsisAntS_getStatusData(ISIS_TRXVU_I2C_BUS_INDEX, ant_side, &status);
	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &status, sizeof(status));
	}

	return err;
}

int CMD_AntGetUptime(sat_packet_t *cmd)
{
	int err = 0;
	time_unix uptime = 0;
	ISISantsSide ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));
	err = IsisAntS_getUptime(ISIS_TRXVU_I2C_BUS_INDEX, ant_side,(unsigned int*) &uptime);
	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &uptime, sizeof(uptime));
	}
	return err;
}

int CMD_StopReDeployment(sat_packet_t *cmd){
	Boolean flag = TRUE;
	int err = 0;
	FRAM_write((unsigned char*) &flag,STOP_REDEPOLOY_FLAG_ADDR, STOP_REDEPOLOY_FLAG_SIZE);

	err = IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX , isisants_sideA, isisants_disarm);
	logError(err ,"CMD_StopReDeployment-IsisAntS_setArmStatus-A");
	err = IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX , isisants_sideB, isisants_disarm);
	logError(err ,"CMD_StopReDeployment-IsisAntS_setArmStatus-B");

	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}
	return err;
}

int CMD_AntCancelDeployment(sat_packet_t *cmd)
{
	int err = 0;
	ISISantsSide ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));
	err = IsisAntS_cancelDeployment(ISIS_TRXVU_I2C_BUS_INDEX, ant_side);
	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}
	return err;
}



