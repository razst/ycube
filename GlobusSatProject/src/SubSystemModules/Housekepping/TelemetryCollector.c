#include <hcc/api_fat.h>

#include "GlobalStandards.h"

#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/Timing/Time.h>

#include <string.h>

#include "TelemetryCollector.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"
#include "SubSystemModules/Maintenance/Maintenance.h"

int GetTelemetryFilenameByType(tlm_type_t tlm_type, char filename[MAX_F_FILE_NAME_SIZE])
{
	return 0;
}

void TelemetryCollectorLogic()
{
}

void TelemetrySaveEPS()
{
}

void TelemetrySaveTRXVU()
{
}

void TelemetrySaveANT()
{
}

void TelemetrySaveSolarPanels()
{
}

void TelemetrySaveWOD()
{
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
	ieps_statcmd_t cmd;
	ieps_enghk_data_mb_t hk_tlm;
	ieps_enghk_data_cdb_t hk_tlm_cdb;
	ieps_board_t board = ieps_board_cdb1;

	err =  IsisEPS_getEngHKDataCDB(EPS_I2C_BUS_INDEX, board, &hk_tlm_cdb, &cmd);
	err += IsisEPS_getRAEngHKDataMB(EPS_I2C_BUS_INDEX, &hk_tlm, &cmd);

	if(err == 0){
		wod->vbat = hk_tlm_cdb.fields.bat_voltage;
		wod->current_3V3 = hk_tlm.fields.obus3V3_curr;
		wod->current_5V = hk_tlm.fields.obus5V_curr;
		wod->volt_3V3 = hk_tlm.fields.obus3V3_volt;
		wod->volt_5V = hk_tlm.fields.obus5V_volt;
		wod->charging_power = hk_tlm.fields.pwr_generating;
		wod->consumed_power = hk_tlm.fields.pwr_delivering;
	}else
		logError(err);

	FRAM_read((unsigned char*)&wod->number_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);

}

