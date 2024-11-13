
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include <utils.h>


#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif


channel_t g_system_state;
EpsState_t state;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode()
{

	if(state == FullMode){
		return 0;
	}
	state = FullMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterCruiseMode()
{
	if(state == CruiseMode){
		return 0;
	}
	state = CruiseMode;
	turnOffTransponder();

	return 0;
}

int EnterSafeMode()
{
	if(state == SafeMode){
		return 0;
	}
	state = SafeMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterCriticalMode()
{
	if(state == CriticalMode){
		return 0;
	}

	state = CriticalMode;
	EpsSetLowVoltageFlag(TRUE);
	return 0;
}

int PayloadOperations(int status)
{
	uint8_t index = (unsigned char)PAYLOAD_SWITCH;
	isis_eps__outputbuschannelon__from_t response;
	int err;
	switch(status)
	{
	case 0: ;//turn on

		isis_eps__outputbuschannelon__to_t param_struct_0;
		param_struct_0.fields.obc_idx = PAYLOAD_SWITCH;
		err = isis_eps__outputbuschannelon__tmtc(index, &param_struct_0, &response);
		break;
	case 1: ;//turn off

		isis_eps__outputbuschanneloff__to_t param_struct_1;
		param_struct_1.fields.obc_idx = PAYLOAD_SWITCH;
		err = isis_eps__outputbuschanneloff__tmtc(index, &param_struct_1, &response);
		break;
	case 2: //restart
		/*
		PayloadOperations(1);
		PayloadOperations(0);*/
		break;
	}
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

