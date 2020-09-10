#include <hcc/api_fat.h>

#include "GlobalStandards.h"

#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
#include <hal/Drivers/ADC.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/Timing/Time.h>
#include "utils.h"

#include <string.h>

#include "TelemetryCollector.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"
#include "FRAM_FlightParameters.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Communication/AckHandler.h"

static time_unix tlm_save_periods[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS] = {0};
static time_unix tlm_last_save_time[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS]= {0};

void InitSavePeriodTimes(){

	FRAM_read((unsigned char*) &tlm_save_periods[tlm_eps], EPS_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
	//printf("tlm_eps period value:%d \n",tlm_save_periods[tlm_eps]);

	FRAM_read((unsigned char*) &tlm_save_periods[tlm_tx], TRXVU_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
	//printf("tlm_tx period value:%d \n",tlm_save_periods[tlm_tx]);

	FRAM_read((unsigned char*) &tlm_save_periods[tlm_antenna], ANT_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
	//printf("tlm_antenna period value:%d \n",tlm_save_periods[tlm_antenna]);

	FRAM_read((unsigned char*) &tlm_save_periods[tlm_solar], SOLAR_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
	//printf("tlm_solar period value:%d \n",tlm_save_periods[tlm_solar]);

	FRAM_read((unsigned char*) &tlm_save_periods[tlm_wod], WOD_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
	//printf("tlm_wod period value:%d \n",tlm_save_periods[tlm_wod]);

}



int CMD_GetTLMPeriodTimes(sat_packet_t *cmd)
{
	int err = TransmitDataAsSPL_Packet(cmd, (unsigned char*) &tlm_save_periods,
				sizeof(time_unix)*NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS);
	return err;
}


int CMD_SetTLMPeriodTimes(sat_packet_t *cmd){

	tlm_type_t tlm_type=cmd->data[0];
	time_unix value=0;
	memcpy(&value,cmd->data+1,sizeof(value));

	int err=0;
	switch (tlm_type)
	{
	case tlm_eps:
		err=FRAM_write((unsigned char *)&value, EPS_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
		tlm_save_periods[tlm_eps] = value;
		break;
	case tlm_tx:
		err=FRAM_write((unsigned char *)&value, TRXVU_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
		tlm_save_periods[tlm_tx] = value;
		break;
	case tlm_antenna:
		err=FRAM_write((unsigned char *)&value, ANT_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
		tlm_save_periods[tlm_antenna] = value;
		break;
	case tlm_solar:
		err=FRAM_write((unsigned char *)&value, SOLAR_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
		tlm_save_periods[tlm_solar] = value;
		break;
	case tlm_wod:
		err=FRAM_write((unsigned char *)&value, WOD_SAVE_TLM_PERIOD_ADDR, sizeof(time_unix));
		tlm_save_periods[tlm_wod] = value;
		break;

	default:
		err=INVALID_TLM_TYPE;
		break;
	}
	if (err == E_NO_SS_ERR){
		SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
	}

	return err;


}

void TelemetryCollectorLogic()
{
	time_unix curr = 0;
	if (CheckExecutionTime(tlm_last_save_time[tlm_eps],tlm_save_periods[tlm_eps])){
		TelemetrySaveEPS();
		if (logError(Time_getUnixEpoch(&curr),"TelemetryCollectorLogic-Time_getUnixEpoch") == 0 ){
			tlm_last_save_time[tlm_eps] = curr;
		}

	}

	if (CheckExecutionTime(tlm_last_save_time[tlm_tx],tlm_save_periods[tlm_tx])){
		TelemetrySaveTRXVU();
		if (logError(Time_getUnixEpoch(&curr),"TelemetryCollectorLogic-Time_getUnixEpoch") == 0 ){
			tlm_last_save_time[tlm_tx] = curr;
		}
	}

	if (CheckExecutionTime(tlm_last_save_time[tlm_antenna],tlm_save_periods[tlm_antenna])){
		TelemetrySaveANT();
		if (logError(Time_getUnixEpoch(&curr),"TelemetryCollectorLogic-Time_getUnixEpoch") == 0 ){
			tlm_last_save_time[tlm_antenna] = curr;
		}
	}

	if (CheckExecutionTime(tlm_last_save_time[tlm_solar],tlm_save_periods[tlm_solar])){
		TelemetrySaveSolarPanels();
		if (logError(Time_getUnixEpoch(&curr),"TelemetryCollectorLogic-Time_getUnixEpoch") == 0 ){
			tlm_last_save_time[tlm_solar] = curr;
		}
	}

	if (CheckExecutionTime(tlm_last_save_time[tlm_wod],tlm_save_periods[tlm_wod])){
		TelemetrySaveWOD();
		if (logError(Time_getUnixEpoch(&curr),"TelemetryCollectorLogic-Time_getUnixEpoch") == 0 ){
			tlm_last_save_time[tlm_wod] = curr;
		}
	}


}

void TelemetrySaveEPS()
{
	isis_eps__gethousekeepingeng__from_t tlm_mb_eng;

	if (logError(isis_eps__gethousekeepingeng__tm(EPS_I2C_BUS_INDEX, &tlm_mb_eng) ,"TelemetrySaveEPS-isis_eps__gethousekeepingeng__tm") == 0)
	{
		write2File(&tlm_mb_eng , tlm_eps);
	}


	/* to save space & time, we only store tlm_eps_eng_mb
 	isis_eps__gethousekeepingraw__from_t tlm_mb_raw;

	if (logError(isis_eps__gethousekeepingraw__tm(EPS_I2C_BUS_INDEX, &tlm_mb_raw)) == 0)
		write2File(&tlm_mb_raw,tlm_eps_raw_mb);

	isis_eps__gethousekeepingrawincdb__from_t tlm_cdb_raw;

	if (logError(isis_eps__gethousekeepingrawincdb__tm(EPS_I2C_BUS_INDEX,&tlm_cdb_raw)) == 0)
	{
		write2File(&tlm_cdb_raw , tlm_eps_raw_cdb);
	}

	isis_eps__gethousekeepingengincdb__from_t tlm_cdb_eng;

	if (logError(isis_eps__gethousekeepingengincdb__tm(EPS_I2C_BUS_INDEX, &tlm_cdb_eng)) == 0)
	{
		write2File(&tlm_cdb_eng , tlm_eps_eng_cdb);
	}
*/

}

void TelemetrySaveTRXVU()
{

		ISIStrxvuTxTelemetry tx_tlm;

		if (logError(IsisTrxvu_tcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX, &tx_tlm) ,"TelemetrySaveTRXVU-IsisTrxvu_tcGetTelemetryAll")== 0)
		{
			write2File(&tx_tlm , tlm_tx);
		}


		ISIStrxvuRxTelemetry rx_tlm;

		if (logError(IsisTrxvu_rcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX, &rx_tlm) ,"TelemetrySaveTRXVU-IsisTrxvu_rcGetTelemetryAll") == 0)
		{
			write2File(&rx_tlm , tlm_rx);
		}

	}

	void TelemetrySaveANT()
	{
		int err = 0;
		ISISantsTelemetry ant_tlmA, ant_tlmB;
		if(logError(IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA,
				&ant_tlmA) ,"TelemetrySaveANT-IsisAntS_getAlltelemetry-A" )==0){
			write2File(&ant_tlmA , tlm_antenna);
		}
		if(logError(IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB,
				&ant_tlmB),"TelemetrySaveANT-IsisAntS_getAlltelemetry-B") == 0){
			write2File(&ant_tlmB , tlm_antenna);
		}
	}

void TelemetrySaveSolarPanels()
{
	//solar_tlm_t data;
	int32_t data[ISIS_SOLAR_PANEL_COUNT] = {0} ;
	int err = 0;
	uint8_t fault;
	if (IsisSolarPanelv2_getState() == ISIS_SOLAR_PANEL_STATE_AWAKE)
	{
		err =  IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0, &data[0],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_1, &data[1],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_2, &data[2],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_3, &data[3],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_4, &data[4],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_5, &data[5],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_6, &data[6],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_7, &data[7],
				&fault);
		err += IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_8, &data[8],
				&fault);

		write2File(&data,tlm_solar);// write the temp values in all cases, even if one was failing
		/*
		if (err == ISIS_SOLAR_PANEL_STATE_AWAKE * ISIS_SOLAR_PANEL_COUNT)
		{
			write2File(&data,tlm_solar);
		}*/
	}
}

void TelemetrySaveWOD()
{
	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry(&wod);
	write2File(&wod , tlm_wod);
}

void GetCurrentWODTelemetry(WOD_Telemetry_t *wod)
{
	if (NULL == wod){
		return;
	}

	memset(wod,0,sizeof(*wod));
	int err = 0;

	FN_SPACE space = { 0 };
	int drivenum = f_getdrive();

	// get the free space of the SD card
	err = f_getfreespace(drivenum, &space);

	if (err == F_NO_ERROR){
		wod->free_memory = space.free;
		wod->corrupt_bytes = space.bad;
	}else
		logError(err ,"GetCurrentWODTelemetry");

	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);
	wod->sat_time = current_time;
	isis_eps__gethousekeepingeng__from_t hk_tlm;
	isis_eps__gethousekeepingengincdb__from_t hk_tlm_cdb;

	err =  isis_eps__gethousekeepingengincdb__tm(EPS_I2C_BUS_INDEX, &hk_tlm_cdb);
	err += isis_eps__gethousekeepingeng__tm(EPS_I2C_BUS_INDEX, &hk_tlm);

	if(err == 0){

		wod->electric_current = hk_tlm.fields.vip_obc00.fields.current;
		wod->vbat = hk_tlm_cdb.fields.dist_input.fields.volt;
		wod->current_3V3 = hk_tlm.fields.vip_obc05.fields.current;
		wod->current_5V = hk_tlm.fields.vip_obc01.fields.current;
		wod->volt_3V3 = hk_tlm.fields.vip_obc05.fields.volt;
		wod->volt_5V = hk_tlm.fields.vip_obc01.fields.volt;
		wod->mcu_temp = hk_tlm.fields.temp;
		wod->bat_temp = hk_tlm.fields.temp3;
		wod->mcu_temp = hk_tlm.fields.temp;
		wod->charging_power = hk_tlm_cdb.fields.batt_input.fields.volt;
		wod->consumed_power = hk_tlm_cdb.fields.dist_input.fields.power;
		// set all solar panels temp values
		uint8_t status;
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0,&wod->solar_panels[0],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_1,&wod->solar_panels[1],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_2,&wod->solar_panels[2],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_3,&wod->solar_panels[3],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_4,&wod->solar_panels[4],&status);
		IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_5,&wod->solar_panels[5],&status);
	}else
		logError(err ,"GetCurrentWODTelemetry");

	int reset=0;
	err = FRAM_read(&reset,NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
	wod->number_of_resets = reset;
	err = FRAM_read(&reset,NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);
	wod->num_of_cmd_resets = reset;

	wod->sat_uptime = Time_getUptimeSeconds();

	// get ADC channels vlaues (include the photo diodes mV values)
	unsigned short adcSamples[8];

	ADC_SingleShot( adcSamples );

	for(int i=0; i <= 4; i++ )
	{
		wod->photo_diodes[i] = ADC_ConvertRaw10bitToMillivolt( adcSamples[i] ); // convert to mV data
		////printf("PD%d : %u mV\n\r", i, wod->photo_diodes[i]);
	}


}

