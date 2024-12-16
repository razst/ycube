
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include <utils.h>

#ifdef ISISEPS
	#include <satellite-subsystems/isismepsv2_ivid5_piu_types.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

#include "SubSystemModules/Housekepping/Payload.h"
#include <satellite-subsystems/common_types.h>


channel_t g_system_state;
EpsState_t state = CriticalMode;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode()
{

	if(state == FullMode){
		return 0;
	}
	state = FullMode;
	state_changed = TRUE;
	EpsSetLowVoltageFlag(FALSE);
	PayloadOperations(TurnOn);
	return 0;
}

int EnterCruiseMode()
{
	if(state == CruiseMode){
		return 0;
	}
	state = CruiseMode;
	state_changed = TRUE;
	turnOffTransponder();
	PayloadOperations(TurnOff);

	return 0;
}

int EnterSafeMode()
{
	if(state == SafeMode){
		return 0;
	}
	state = SafeMode;
	state_changed = TRUE;
	EpsSetLowVoltageFlag(FALSE);
	PayloadOperations(TurnOff);
	return 0;
}

int EnterCriticalMode()
{
	if(state == CriticalMode){
		return 0;
	}

	state = CriticalMode;
	state_changed = TRUE;
	EpsSetLowVoltageFlag(TRUE);
	PayloadOperations(TurnOff);
	return 0;
}

int PayloadOperations(PayloadOperation status)
{
	uint8_t index = 0;
	isismepsv2_ivid5_piu__replyheader_t response;
	int err = 0;

	switch(status)
	{
	case TurnOn: ;

		if(!logError(isismepsv2_ivid5_piu__outputbuschannelon(index, PAYLOAD_SWITCH, &response), "Turn on payload channel")){return -1;}

		//increase the number of sw3 resets
		unsigned int num_of_resets = 0;
		FRAM_read((unsigned char*) &num_of_resets,
		NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);
		num_of_resets++;

		FRAM_write((unsigned char*) &num_of_resets,
		NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);
		break;

	case TurnOff: ;

		if(logError(isismepsv2_ivid5_piu__outputbuschanneloff(index, PAYLOAD_SWITCH, &response), "Turn off payload channel")){return -1;}
		break;

	case Restart: ;

		err = PayloadOperations(TurnOff);
		vTaskDelay(10);
		err = PayloadOperations(TurnOn);
		break;
	}
	return err;
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

