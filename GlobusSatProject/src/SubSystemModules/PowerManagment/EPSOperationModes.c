
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include <utils.h>

#ifdef ISISEPS
	#include <satellite-subsystems/isismepsv2_ivid7_piu.h>
	#include <satellite-subsystems/isismepsv2_ivid7_piu_types.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

#include "SubSystemModules/Communication/TRXVU.h"
#include <satellite-subsystems/common_types.h>


channel_t g_system_state;
EpsState_t state;// = CriticalMode;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode()
{
	int err;
	/*if(state == FullMode){
		return 0;
	}*/
	state = FullMode;
	EpsSetLowVoltageFlag(FALSE);
	err = Payload_Safety();
	//PayloadOperations(TurnOn);
	return err;
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
	Boolean isOn = DoesPayloadChannelOn();
	uint8_t index = 0;
	isismepsv2_ivid7_piu__replyheader_t response;
	int err = 0;

	switch(status)
	{
	case TurnOn: ;
		if(isOn){return PAYLOAD_FALSE_OPERATION;}
		if(logError(isismepsv2_ivid7_piu__outputbuschannelon(index, isismepsv2_ivid7_piu__imeps_channel__channel_5v_sw3, &response), "Turn on payload channel")){return -1;}

		//increase the number of sw3 resets
		unsigned int num_of_resets = 0;
		FRAM_read((unsigned char*) &num_of_resets,
		NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);
		num_of_resets++;

		FRAM_write((unsigned char*) &num_of_resets,
		NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);
		break;

	case TurnOff: ;
		if(!isOn){return PAYLOAD_FALSE_OPERATION;}
		if(logError(isismepsv2_ivid7_piu__outputbuschanneloff(index, isismepsv2_ivid7_piu__imeps_channel__channel_5v_sw3, &response), "Turn off payload channel")){return -1;}

		break;

	case Restart: ; //quest - allow this when the channel is off?

		err = PayloadOperations(TurnOff);
		vTaskDelay(900);
		err = PayloadOperations(TurnOn);
		break;
	}
	return err;
}
int Payload_Safety()
{
	Boolean PayloadState;
	Boolean Has_Sat_Reset;
	FRAM_read((unsigned char*)&PayloadState,Payload_IS_Dead_ADDR,Payload_IS_Dead_SIZE);
	if(PayloadState != TRUE)
	{
		FRAM_read((unsigned char*)&Has_Sat_Reset,Has_Sat_Reset_ADDR,Has_Sat_Reset_SIZE);
		if (Has_Sat_Reset == TRUE)
		{
			PayloadState = TRUE;
			FRAM_write((unsigned char*)&PayloadState,Payload_IS_Dead_ADDR,Payload_IS_Dead_SIZE);
		}
		else 
		{
			Has_Sat_Reset = TRUE;
			FRAM_write((unsigned char*)&Has_Sat_Reset,Has_Sat_Reset_ADDR,Has_Sat_Reset_SIZE);
			PayloadOperations(TurnOn);
			return E_NO_SS_ERR;
		}
	}
	else return -1;
}
void Payload_Safety_IN_Maintenance()
{
	Boolean Has_Sat_Reset;
	FRAM_read((unsigned char*)&Has_Sat_Reset,Has_Sat_Reset_ADDR,Has_Sat_Reset_SIZE);
	/*time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);*/
	//if uptime > 1 min 
	if(Time_getUptimeSeconds() > 60)
	{
		Has_Sat_Reset = FALSE;
		FRAM_write(&Has_Sat_Reset,Has_Sat_Reset_ADDR,Has_Sat_Reset_SIZE);
	}
}
Boolean DoesPayloadChannelOn()
{
	uint8_t index = 0;
	isismepsv2_ivid7_piu__gethousekeepingeng__from_t response;

	if(logError(isismepsv2_ivid7_piu__gethousekeepingeng(index, &response), "get Housekeeping Data - check for payload channel")){return FALSE;}

	//TODO - how much should we check? when there is no power there it can still show some numbers so change 0 to smth
	if(response.fields.vip_obc04.fields.volt > 0 && response.fields.vip_obc04.fields.current > 0 && response.fields.vip_obc04.fields.power > 0)
	{
		return TRUE;
	}

	return FALSE;
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

