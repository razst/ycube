#include <satellite-subsystems/IsisAntS.h>


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
	//TODO: finish 'trxvu_command_router'
	switch (cmd->cmd_subtype)
	{
	case DUMP_SUBTYPE:
		err = CMD_StartDump(cmd);
		break;

	case ABORT_DUMP_SUBTYPE:
		err = CMD_SendDumpAbortRequest(cmd);
		break;

	case FORCE_ABORT_DUMP_SUBTYPE:
		err = CMD_ForceDumpAbort(cmd);
		break;

	case MUTE_TRXVU:
		err = CMD_MuteTRXVU(cmd);
		break;

	case UNMUTE_TRXVU:
		err = CMD_UnMuteTRXVU(cmd);
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

	case SET_BAUD_RATE:
		err = CMD_SetBaudRate(cmd);
		break;

	case GET_TX_UPTIME:
		err = CMD_GetTxUptime(cmd);
		break;

	case GET_RX_UPTIME:
		err = CMD_GetRxUptime(cmd);
		break;

	case GET_NUM_OF_DELAYED_CMD: // TODO
		err = CMD_GetNumOfDelayedCommands(cmd);
		break;

	case GET_NUM_OF_ONLINE_CMD:
		err = CMD_GetNumOfOnlineCommands(cmd);
		break;
	case ADD_DELAYED_COMMAND_CMD:
		ParseDataToCommand(cmd->data,&delayed_cmd);
		err = AddDelayedCommand(&delayed_cmd);
		break;
	case DELETE_DELAYED_CMD:
		err = CMD_DeleteDelyedCmdByID(cmd);
		break;

	case DELETE_ALL_DELAYED_CMD:
		err = CMD_DeleteAllDelyedBuffer(cmd);
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
	//TODO: finish 'eps_command_router'
	int err = 0;

	switch (cmd->cmd_subtype)
	{
	case 0:
		err = UpdateAlpha(*(float*)cmd->data);
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
	return 0;
}

int managment_command_router(sat_packet_t *cmd)
{
	return 0;
}

int filesystem_command_router(sat_packet_t *cmd)
{
	return 0;
}
