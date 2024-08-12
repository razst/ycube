
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
	int err =0;
	if(state == CruiseMode){
		return 0;
	}
	state = CruiseMode;

	char data[2] = {0,0};
	data[0] = 0x38;
	data[1] = 0x01;//nominal

	err = I2C_write(I2C_TRXVU_TC_ADDR, data, 2);
	setTransponderEndTime(0);
	EpsSetLowVoltageFlag(FALSE);
	return err;
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

