
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
	PayloadOperations(TurnOn);
	return 0;
}

int EnterCruiseMode()
{
	if(state == CruiseMode){
		return 0;
	}
	state = CruiseMode;
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
	EpsSetLowVoltageFlag(TRUE);
	PayloadOperations(TurnOff);
	return 0;
}

int PayloadOperations(PayloadOperation status)
{
	uint8_t index = (unsigned char)PAYLOAD_SWITCH;
	isis_eps__outputbuschannelon__from_t response;
	int err = 0;

	switch(status)
	{
	case TurnOn: ;

		isis_eps__outputbuschannelon__to_t param_struct_0;
		param_struct_0.fields.obc_idx = PAYLOAD_SWITCH;
		//err = isis_eps__outputbuschannelon__tmtc(index, &param_struct_0, &response);

		//TODO - check response

		//increase the number of sw3 resets
		unsigned int num_of_resets = 0;
		FRAM_read((unsigned char*) &num_of_resets,
		NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);
		num_of_resets++;

		FRAM_write((unsigned char*) &num_of_resets,
		NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);
		break;

	case TurnOff: ;

		isis_eps__outputbuschanneloff__to_t param_struct_1;
		param_struct_1.fields.obc_idx = PAYLOAD_SWITCH; // change to CONST
		//err = isis_eps__outputbuschanneloff__tmtc(index, &param_struct_1, &response);

		//TODO - check response

		break;

	case Restart: ;

		err = PayloadOperations(1);
		vTaskDelay(10);
		err = PayloadOperations(0);
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

