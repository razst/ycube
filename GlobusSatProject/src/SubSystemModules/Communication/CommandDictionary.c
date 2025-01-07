#include <satellite-subsystems/isis_ants_rev2.h>


#include "SubSystemModules/Communication/SubsystemCommands/TRXVU_Commands.h"
#include "SubSystemModules/Communication/SubsystemCommands/Maintanence_Commands.h"
#include "SubSystemModules/Communication/SubsystemCommands/FS_Commands.h"
#include "SubSystemModules/Communication/SubsystemCommands/EPS_Commands.h"

#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "TLM_management.h"
#include <stdio.h>
#include "CommandDictionary.h"

int trxvu_command_router(sat_packet_t *cmd)
{
	int err = 0;
	sat_packet_t delayed_cmd = {0};
	switch (cmd->cmd_subtype)
	{
	case PING: 								//this command is a ping function
		SendAckPacket(ACK_PING, cmd,NULL,0);
		break;

	case DUMP_DAYS:
		err = CMD_StartDump(cmd);
		break;

	case DUMP_TIME_RANGE:
		err = CMD_StartDump(cmd);
		break;

	case DUMP_RAM_TLM:
		err = CMD_DumpRamTLM(cmd);
		break;

	case ABORT_DUMP_SUBTYPE:
		err = CMD_SendDumpAbortRequest(cmd);
		break;

//	case FORCE_ABORT_DUMP_SUBTYPE: Note: when sending this cmd, the sat hangs, so we comment this out
//		err = CMD_ForceDumpAbort(cmd);
//		break;

	case SET_TRANSPONDER:
		err = CMD_SetTransponder(cmd);
		break;

	case SET_RSSI_TRANSPONDER:
		err = CMD_SetRSSITransponder(cmd);
		break;

	case MUTE_TRXVU:
		err = CMD_MuteTRXVU(cmd);
		break;

	case UNMUTE_TRXVU:
		err = CMD_UnMuteTRXVU(cmd);
		break;

	case TRXVU_IDLE:
		err = CMD_SetIdleState(cmd);
		break;

	case GET_BAUD_RATE:
		err = CMD_GetBaudRate(cmd);
		break;

	case GET_BEACON_INTERVAL:
		err = CMD_GetBeaconInterval(cmd);
		break;

	case SET_BEACON_INTERVAL:
		err = CMD_SetBeaconInterval(cmd);
		break;

	case TRANSMIT_BEACON:
		err = CMD_TrasmitBeacon(cmd);
		break;

	case SET_BAUD_RATE:
		err = CMD_SetBaudRate(cmd);
		break;

	case GET_TX_UPTIME:
		err = CMD_GetTxUptime(cmd);
		break;

	case GET_RX_UPTIME:
		err = CMD_GetRxUptime(cmd);
		break;

	case GET_NUM_OF_ONLINE_CMD:
		err = CMD_GetNumOfOnlineCommands(cmd);
		break;

	case ANT_SET_ARM_STATUS:
		err = CMD_AntSetArmStatus(cmd);
		break;

	case ANT_GET_ARM_STATUS:
		err = CMD_AntGetArmStatus(cmd);
		break;

	case ANT_GET_UPTIME:
		err = CMD_AntGetUptime(cmd);
		break;

	case ANT_CANCEL_DEPLOY:
		err = CMD_AntCancelDeployment(cmd);
		break;

	case ANT_DEPLOY:
		err = CMD_AntennaDeploy(cmd);
		break;

	case ANT_STOP_REDEPLOY:
		err = CMD_StopReDeployment(cmd);
		break;

	case SECURED_CMD:
		err = CMD_SecurePing(cmd);
		break;

	default:
		err = SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}

	if (err != 0) {
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char*) &err, sizeof(err));
	}

	return err;
}

int eps_command_router(sat_packet_t *cmd)
{

	int err = 0;

	switch (cmd->cmd_subtype)
	{
	case UPDATE_ALPHA:
		err = UpdateAlpha(cmd);
		break;
	case GET_HEATER_VALUES:
		err = CMDGetHeaterValues(cmd);
		break;
	case SET_HEATER_VALUES:
		err = CMDSetHeaterValues(cmd);
		break;
	case RESET_EPS_WDT:
		err = CMD_EPS_ResetWDT(cmd);
		break;
	case PAYLOAD_OPERATIONS:
		err = CMD_Payload_Operations(cmd);
		break;
	case ENABLE_PAYLOAD:
		err = CMD_Change_Payload_State_INFRAM(cmd);
		break;
	default:
		SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}
	if (err != 0) {
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char*) &err, sizeof(err));
	}
	return err;

}

int telemetry_command_router(sat_packet_t *cmd)
{
	int err = 0;

	switch (cmd->cmd_subtype)
	{
	case DELETE_FILE:
		err = CMD_DeleteTLM(cmd);
		break;
	case DELETE_ALL_FILES:
		err = CMD_DeleteAllFiles(cmd);
		break;
	case GET_LAST_FS_ERROR:
		err = CMD_GetLastFS_Error(cmd);
		break;
	case SET_TLM_PERIOD:
		err = CMD_SetTLMPeriodTimes(cmd);
		break;
	case GET_TLM_PERIOD:
		err = CMD_GetTLMPeriodTimes(cmd);
		break;
	case GET_IMAGE_INFO:
		err = CMD_getInfoImage(cmd);
		break;
	case GET_IMAGE_DATA:
		err = CMD_getDataImage(cmd);
		break;
	case GET_TLM_INFO:
		err = CMD_Get_TLM_Info(cmd);
		break;
	case SWITCH_SD_CARD:
		err = CMD_Switch_SD_Card(cmd);
		break;
	case FORMAT_SD_CARD:
		err = CMD_Format_SD_Card(cmd);
		break;

	default:
		err = SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}

	if (err != 0) {
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char*) &err, sizeof(err));
	}

	return err;
}

int managment_command_router(sat_packet_t *cmd)
{
	int err = 0;

	switch (cmd->cmd_subtype)
	{
	case RESET_COMPONENT:
		err = CMD_ResetComponent(cmd);
		break;

	case UPDATE_SAT_TIME:
		err = CMD_UpdateSatTime(cmd);
		break;

	case GENERIC_I2C_CMD:
		err = CMD_GenericI2C(cmd);
		break;

	case FRAM_WRITE_AND_TRANSMIT:
		err = CMD_FRAM_WriteAndTransmitt(cmd);
		break;

	case FRAM_READ_AND_TRANSMIT:
		err = CMD_FRAM_ReadAndTransmitt(cmd);
		break;
	case FRAM_RESTART:
		err = CMD_FRAM_ReStart(cmd);
		break;
	case GET_SAT_UPTIME:
		err = CMD_GetSatUptime(cmd);
		break;
	case GET_DEV_INFO:
		err = CMD_GetDevInfo(cmd);
		break;

	default:
		err = SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}

	if (err != 0) {
		SendAckPacket(ACK_ERROR_MSG, cmd, (unsigned char*) &err, sizeof(err));
	}

	return err;
}
int filesystem_command_router(sat_packet_t *cmd)
{
	return 0;
}
