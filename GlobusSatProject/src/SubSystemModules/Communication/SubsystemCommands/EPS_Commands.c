#include "GlobalStandards.h"

#ifdef ISISEPS
	#include <satellite-subsystems/isismepsv2_ivid7_piu.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif


#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <stdlib.h>
#include <string.h>

#include  "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/PowerManagment/EPSOperationModes.h"
#include "EPS_Commands.h"
#include <hal/errors.h>
int CMD_UpdateThresholdVoltages(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetThresholdVoltages(sat_packet_t *cmd)
{
	return 0;
}

int CMD_UpdateSmoothingFactor(sat_packet_t *cmd)
{
	return 0;
}

int CMD_RestoreDefaultAlpha(sat_packet_t *cmd)
{
	return 0;
}

int CMD_RestoreDefaultThresholdVoltages(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetSmoothingFactor(sat_packet_t *cmd)
{
	return 0;
}

int CMD_EnterCruiseMode(sat_packet_t *cmd)
{
	return 0;
}

int CMD_EnterFullMode(sat_packet_t *cmd)
{
	return 0;
}

int CMD_EnterCriticalMode(sat_packet_t *cmd)
{
	return 0;
}

int CMD_EnterSafeMode(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetCurrentMode(sat_packet_t *cmd)
{
	return 0;
}

int CMD_EPS_NOP(sat_packet_t *cmd)
{
	return 0;
}

int CMD_EPS_ResetWDT(sat_packet_t *cmd)
{
	isismepsv2_ivid7_piu__replyheader_t res;
	int err = isismepsv2_ivid7_piu__resetwatchdog(EPS_I2C_BUS_INDEX,&res);
	if (err == E_NO_SS_ERR)
	{
		SendAckPacket(ACK_COMD_EXEC, cmd, NULL, 0);
	}
	return err;
}
int CMD_Payload_Operations (sat_packet_t *cmd)
{
	char state;
	int err;
	PayloadOperation status;
	Boolean isOn = DoesPayloadChannelOn();

	memcpy(&state,cmd -> data,1);
	switch(state)
	{
		case 1:
			if(isOn){return PAYLOAD_FALSE_OPERATION;}
			status = TurnOn;
			break;
		case 2:
			if(!isOn){return PAYLOAD_FALSE_OPERATION;}
			status = TurnOff;
			break;
		case 3://quest - allow this when the channel is off?
			status = Restart;
			break;
		default:
			return E_PARAM_OUTOFBOUNDS;
			break;
	}
	
	err = PayloadOperations(status);
	return err;
}

int CMD_EPS_SetChannels(sat_packet_t *cmd)
{
	return 0;
}

int CMD_SetChannels3V3_On(sat_packet_t *cmd)
{
	return 0;
}

int CMD_SetChannels3V3_Off(sat_packet_t *cmd)
{
	return 0;
}

int CMD_SetChannels5V_On(sat_packet_t *cmd)
{
	return 0;
}

int CMD_SetChannels5V_Off(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetEpsParameter(sat_packet_t *cmd)
{
	return 0;
}

int CMD_SetEpsParemeter(sat_packet_t *cmd)
{
	return 0;
}

int CMD_ResetParameter(sat_packet_t *cmd)
{
	return 0;
}

int CMD_ResetConfig(sat_packet_t *cmd)
{
	return 0;
}

int CMD_LoadConfig(sat_packet_t *cmd)
{
	return 0;
}

int CMD_SaveConfig(sat_packet_t *cmd)
{
	return 0;
}

int CMD_SolarPanelWake(sat_packet_t *cmd)
{
	return 0;
}

int CMD_SolarPanelSleep(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetSolarPanelState(sat_packet_t *cmd)
{
	return 0;
}

