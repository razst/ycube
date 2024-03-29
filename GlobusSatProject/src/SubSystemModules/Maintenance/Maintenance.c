#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hcc/api_fat.h>

#include <hal/Timing/Time.h>
#include "TestingConfigurations.h"
#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

#include "GlobalStandards.h"

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <hcc/api_fat.h>

#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "TLM_management.h"
#include "Maintenance.h"
#include "utils.h"
#include <math.h>


time_unix lastDeploy = 0;

Boolean CheckExecutionTime(time_unix prev_time, time_unix period)
{
	time_unix curr = 0;
	if (logError(Time_getUnixEpoch(&curr) ,"CheckExecutionTime-Time_getUnixEpoch") == 1) return -1;
	if(curr - prev_time >= period){
		return TRUE;
	}
	return FALSE;

}

Boolean CheckExecTimeFromFRAM(unsigned int fram_time_addr, time_unix period)
{
	int err = 0;
	time_unix prev_exec_time = 0;
	err = FRAM_read((unsigned char*)&prev_exec_time,fram_time_addr,sizeof(prev_exec_time));
	if(0 != err){
		return FALSE;
	}
	return CheckExecutionTime(prev_exec_time,period);
}

void SaveSatTimeInFRAM(unsigned int time_addr, unsigned int time_size)
{
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	FRAM_write((unsigned char*) &current_time, time_addr, time_size);
}

Boolean IsFS_Corrupted()
{
	FN_SPACE space;
	int drivenum = f_getdrive();

	f_getfreespace(drivenum, &space);

	if (space.bad > 0) {
		return TRUE;
	}
	return FALSE;
}

int WakeupFromResetCMD()
{
	int err = 0;
	unsigned char reset_flag = 0;
	unsigned int num_of_resets = 0;
	FRAM_read(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);

	// first incfease the number of total resets
	FRAM_read((unsigned char*) &num_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
	num_of_resets++;

	FRAM_write((unsigned char*) &num_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);

	// if we came back from a reset command we got from ground station, increase the number of cmd resets
	if (reset_flag) {
		time_unix curr_time = 0;
		Time_getUnixEpoch(&curr_time);

		err = SendAckPacket(ACK_RESET_WAKEUP, NULL, (unsigned char*) &curr_time,
				sizeof(time_unix));

		reset_flag = FALSE_8BIT;
		FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);

		FRAM_read((unsigned char*) &num_of_resets,
		NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);
		num_of_resets++;

		FRAM_write((unsigned char*) &num_of_resets,
		NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);
		if (0 != err) {
			return err;
		}
	}

	return 0;
}

void ResetGroundCommWDT()
{
	SaveSatTimeInFRAM(LAST_COMM_TIME_ADDR,
			LAST_COMM_TIME_SIZE);
}

// check if last communication with the ground station has passed WDT kick time
// return TRUE if we past wdt_kick_thresh
Boolean IsGroundCommunicationWDTKick()
{
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	time_unix last_comm_time = 0;
	FRAM_read((unsigned char*) &last_comm_time, LAST_COMM_TIME_ADDR,
	LAST_COMM_TIME_SIZE);

	time_unix wdt_kick_thresh = GetGsWdtKickTime();


	if (current_time - last_comm_time >= wdt_kick_thresh) {
		return TRUE;
	}
	return FALSE;
}

int SetGsWdtKickTime(time_unix new_gs_wdt_kick_time)
{
	int err = FRAM_write((unsigned char*)&new_gs_wdt_kick_time, NO_COMM_WDT_KICK_TIME_ADDR,
		NO_COMM_WDT_KICK_TIME_SIZE);
	return err;
}

time_unix GetGsWdtKickTime()
{
	time_unix no_comm_thresh = 0;
	FRAM_read((unsigned char*)&no_comm_thresh, NO_COMM_WDT_KICK_TIME_ADDR,NO_COMM_WDT_KICK_TIME_SIZE);
	return no_comm_thresh;
}


int DeleteOldFiels(int minFreeSpace){
	// check how much free space we have in the SD
	FN_SPACE space = { 0 };
	int drivenum = f_getdrive();

	// get the free space of the SD card

	if (logError(f_getfreespace(drivenum, &space) ,"DeleteOldFiels-f_getfreespace")) return -1;

	// if needed, clean old files
	if (space.free < minFreeSpace){
		Time theDay;
		theDay.year = 20;
		theDay.date = 1;
		theDay.month = 1;
		int numOfDays = 0;
		//read the last numOfDays from FRAM
		FRAM_read((unsigned char*) &numOfDays,
		DEL_OLD_FILES_NUM_DAYS_ADDR, DEL_OLD_FILES_NUM_DAYS_SIZE);

		while (numOfDays < (365*5)){ // just in case that we won't get into endless loop, stop after 5 years
			int err = deleteTLMFile(tlm_wod,theDay,numOfDays);
			err *= deleteTLMFile(tlm_antenna,theDay,numOfDays);
			err *= deleteTLMFile(tlm_eps,theDay,numOfDays);
			err *= deleteTLMFile(tlm_eps_raw_cdb_NOT_USED,theDay,numOfDays);
			err *= deleteTLMFile(tlm_eps_raw_mb_NOT_USED,theDay,numOfDays);
			err *= deleteTLMFile(tlm_log,theDay,numOfDays);
			err *= deleteTLMFile(tlm_rx,theDay,numOfDays);
			err *= deleteTLMFile(tlm_rx_frame,theDay,numOfDays);
			err *= deleteTLMFile(tlm_solar,theDay,numOfDays);
			err *= deleteTLMFile(tlm_tx,theDay,numOfDays);
			// if all files were found & deleted -> break
			// if all files were not found -> don't break, move to next day
			// if found some but not all -> break
			if(err != pow(F_ERR_NOTFOUND,12)){
				break;
			}
			numOfDays++;
		}

		//write the numOfDays into FRAM
		FRAM_write((unsigned char*) &numOfDays,
		DEL_OLD_FILES_NUM_DAYS_ADDR, DEL_OLD_FILES_NUM_DAYS_SIZE);

	}
}

void CheckDeployAnt(){

	Boolean flag;
	FRAM_read((unsigned char*) &flag, STOP_REDEPOLOY_FLAG_ADDR,STOP_REDEPOLOY_FLAG_SIZE);

	// if we don't have any last deploy time, than its our first iteration check, take current time so we don't deploy immediately
	if (lastDeploy==0){
		Time_getUnixEpoch(&lastDeploy);
		//FRAM_read((unsigned char*) &lastDeploy, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE);
	}

	if (!flag  && CheckExecutionTime(lastDeploy,DEPLOY_INTRAVAL)){
		CMD_AntennaDeploy(NULL);
		Time_getUnixEpoch(&lastDeploy);
	}
}

int HardResetMCU(){
	isis_eps__reset__to_t cmd_t;
	isis_eps__reset__from_t cmd_f;
	cmd_t.fields.rst_key = RESET_KEY;
//	logError(isis_eps__reset__tmtc(EPS_I2C_BUS_INDEX, &cmd_t, &cmd_f),"CMD_ResetComponent-isis_eps__reset__tmtc");
}

void Maintenance()
{
	SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);

	//logError(IsFS_Corrupted());-> we send corrupted bytes over beacon, no need to log in error file all the time

	// check if for too long we didn't got any comm from ground, and reset TRXVU and sat if needed
	if (IsGroundCommunicationWDTKick()){
		logError(INFO_MSG,"Maintenance-WDTKick, going to restart systems");
		ResetGroundCommWDT(); // to make sure we don't get into endless restart loop
		// hard reset the TRXVU
		logError(IsisTrxvu_hardReset(ISIS_TRXVU_I2C_BUS_INDEX),"Maintenance-IsisTrxvu_hardReset");
		vTaskDelay(500);
		SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);// store the most updated sat time
		HardResetMCU();
	}

	DeleteOldFiels(MIN_FREE_SPACE);

	CheckDeployAnt();
}

