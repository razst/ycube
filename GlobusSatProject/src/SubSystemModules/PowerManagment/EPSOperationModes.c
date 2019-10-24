
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include <utils.h>


#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif


//TODO: update functions to only the relevant channels
channel_t g_system_state;
EpsState_t state;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode()
{
	if(state == FullMode){
		return 0;
	}
	if(logError(SetEPS_Channels((channel_t)CHNNELS_ON)))return -1;
	state = FullMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterCruiseMode()
{
	if(state == CruiseMode){
		return 0;
	}
	if(logError(SetEPS_Channels((channel_t)CHANNELS_OFF)))return -1;
	state = CruiseMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterSafeMode()
{
	if(state == SafeMode){
		return 0;
	}
	if(logError(SetEPS_Channels((channel_t)CHANNELS_OFF)))return -1;
	state = SafeMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterCriticalMode()
{
	if(state == CriticalMode){
		return 0;
	}
	//TODO check which channels to turn off - is the OBC CONNECTED TO A CHANNEL??? we don't want to turn OBC off !!!
	if(logError(SetEPS_Channels((channel_t)CHANNELS_OFF)))return -1;
	state = CriticalMode;
	EpsSetLowVoltageFlag(TRUE);
	return 0;
}

int SetEPS_Channels(channel_t channel)
{
	ieps_statcmd_t code;
	ieps_obus_channel_t chan;
	//TODO why do we need to turn on all existing g_sys_State? they are already on
	chan.raw = g_system_state;
	if(logError(IsisEPS_outputBusGroupOn(EPS_I2C_BUS_INDEX , chan , chan , &code)))return -1;

	g_system_state = channel;
	chan.raw = ~g_system_state;
	if(logError(IsisEPS_outputBusGroupOff(EPS_I2C_BUS_INDEX , chan , chan , &code)))return -1;

	return 0;
}

EpsState_t GetSystemState()
{
	return state;
}

channel_t GetSystemChannelState()
{
	return g_system_state;
}

Boolean EpsGetLowVoltageFlag()
{
	return g_low_volt_flag;
}

void EpsSetLowVoltageFlag(Boolean low_volt_flag)
{
	g_low_volt_flag = low_volt_flag;
}

