#include <hcc/api_fat.h>

#include "GlobalStandards.h"

#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

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


time_unix tlm_save_periods[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS] = {0};
time_unix tlm_last_save_time[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS]= {0};

void InitSavePeriodTimes(){
	FRAM_read((unsigned char*)tlm_save_periods,TLM_SAVE_PERIOD_START_ADDR,NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS*sizeof(time_unix));
}

void TelemetryCollectorLogic()
{
	if (CheckExecutionTime(tlm_last_save_time[tlm_eps],tlm_save_periods[tlm_eps])){
		TelemetrySaveEPS();
		Time_getUnixEpoch(tlm_last_save_time[tlm_eps]);
	}

	if (CheckExecutionTime(tlm_last_save_time[tlm_tx],tlm_save_periods[tlm_tx])){
		TelemetrySaveTRXVU();
		Time_getUnixEpoch(tlm_last_save_time[tlm_tx]);
	}

	if (CheckExecutionTime(tlm_last_save_time[tlm_antenna],tlm_save_periods[tlm_antenna])){
		TelemetrySaveANT();
		Time_getUnixEpoch(tlm_last_save_time[tlm_antenna]);
	}

	if (CheckExecutionTime(tlm_last_save_time[tlm_solar],tlm_save_periods[tlm_solar])){
		TelemetrySaveSolarPanels();
		Time_getUnixEpoch(tlm_last_save_time[tlm_solar]);
	}

	if (CheckExecutionTime(tlm_last_save_time[tlm_wod],tlm_save_periods[tlm_wod])){
		TelemetrySaveWOD();
		Time_getUnixEpoch(tlm_last_save_time[tlm_wod]);
	}


}

void TelemetrySaveEPS()
{

	isis_eps__gethousekeepingraw__from_t tlm_mb_raw;

	if (logError(isis_eps__gethousekeepingraw__tm(EPS_I2C_BUS_INDEX, &tlm_mb_raw)) == 0)
		write2File(&tlm_mb_raw,tlm_eps_raw_mb);

	isis_eps__gethousekeepingeng__from_t tlm_mb_eng;

	if (logError(isis_eps__gethousekeepingeng__tm(EPS_I2C_BUS_INDEX, &tlm_mb_eng)) == 0)
	{
		write2File(&tlm_mb_eng , tlm_eps_eng_cdb);
	}

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

}

void TelemetrySaveTRXVU()
{

		ISIStrxvuTxTelemetry tx_tlm;

		if (logError(IsisTrxvu_tcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX, &tx_tlm))== 0)
		{
			write2File(&tx_tlm , tlm_tx);
		}

		ISIStrxvuTxTelemetry_revC revc_tx_tlm;

		if (logError(IsisTrxvu_tcGetTelemetryAll_revC(ISIS_TRXVU_I2C_BUS_INDEX,
				&revc_tx_tlm)) == 0)
		{
			write2File(&revc_tx_tlm , tlm_tx_revc);
		}

		ISIStrxvuRxTelemetry rx_tlm;

		if (logError(IsisTrxvu_rcGetTelemetryAll(ISIS_TRXVU_I2C_BUS_INDEX, &rx_tlm)) == 0)
		{
			write2File(&rx_tlm , tlm_rx);
		}

		ISIStrxvuRxTelemetry_revC revc_rx_tlm;
		if (logError(IsisTrxvu_rcGetTelemetryAll_revC(ISIS_TRXVU_I2C_BUS_INDEX,
				&revc_rx_tlm)) == 0)
		{
			write2File(&revc_rx_tlm , tlm_rx_revc);
		}
	}

	void TelemetrySaveANT()
	{
		int err = 0;
		ISISantsTelemetry ant_tlmA, ant_tlmB;
		if(logError(IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA,
				&ant_tlmA) == 0)){
			write2File(&ant_tlmA , tlm_antenna);
		}
		if(logError(IsisAntS_getAlltelemetry(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB,
				&ant_tlmB)) == 0){
			write2File(&ant_tlmB , tlm_antenna);
		}
	}

void TelemetrySaveSolarPanels()
{
	//solar_tlm_t data;
	int32_t data[ISIS_SOLAR_PANEL_COUNT] ;
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

		if (err == ISIS_SOLAR_PANEL_STATE_AWAKE * ISIS_SOLAR_PANEL_COUNT)
		{
			write2File(&data,tlm_solar);
		}
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
		logError(err);

	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);
	wod->sat_time = current_time;
	isis_eps__gethousekeepingeng__from_t hk_tlm;
	isis_eps__gethousekeepingengincdb__from_t hk_tlm_cdb;

	err =  isis_eps__gethousekeepingengincdb__tm(EPS_I2C_BUS_INDEX, &hk_tlm_cdb);
	err += isis_eps__gethousekeepingeng__tm(EPS_I2C_BUS_INDEX, &hk_tlm);

	if(err == 0){
		// TODO: check if which values do we need to use and are we using the right ones??
		wod->vbat = hk_tlm_cdb.fields.batt_input.fields.volt;
		//wod->current_3V3 = hk_tlm.fields.obus3V3_curr;
		wod->current_5V = hk_tlm_cdb.fields.batt_input.fields.current; // TODO: this is the total current and not the 5V current
		//wod->volt_3V3 = hk_tlm.fields.obus3V3_volt;
		//wod->volt_5V = hk_tlm.fields.obus5V_volt;
		//wod->charging_power = hk_tlm.fields.pwr_generating;
		wod->consumed_power = hk_tlm_cdb.fields.batt_input.fields.power;
	}else
		logError(err);

	FRAM_read((unsigned char*)&wod->number_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);

}

