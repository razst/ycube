#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hcc/api_fat.h>

#include <hal/Timing/Time.h>
#include "TestingConfigurations.h"
#ifdef ISISEPS
	#include <satellite-subsystems/isismepsv2_ivid7_piu.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

#include "GlobalStandards.h"

#include <satellite-subsystems/isis_vu_e.h>
#include <satellite-subsystems/isis_ants_rev2.h>

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


unsigned short* findMinMaxDate(){

	char* path = "TLM/*.*";
	unsigned short minMaxMonth[2] = { 99999, 0 };
	F_FIND find;
	if (!f_findfirst(path,&find)) {
			do {
				char* filename = find.filename;
				if (filename[0] != '.')
				{
					int month = atoi(filename);
					if (minMaxMonth[0] > month)
						minMaxMonth[0] = month;
					if (minMaxMonth[1] < month)
						minMaxMonth[1] = month;
				}
			} while (!f_findnext(&find));
	}
	return minMaxMonth;
}


int DeleteOldFiels(int minFreeSpace){
	// check how much free space we have in the SD
	FN_SPACE space = { 0 };
	int drivenum = f_getdrive();

	// get the free space of the SD card

	if (logError(f_getfreespace(drivenum, &space) ,"DeleteOldFiels-f_getfreespace")) return -1;

	// if needed, clean old files
	if (space.free < minFreeSpace)
	{
		return deleteTLMbyMonth(findMinMaxDate()[0]);
	}
	return E_NO_SS_ERR;
}

// every DEPLOY_INTRAVAL time, we call for ANT deploy - forever - just in case and to make sure we have open ANTS
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
//	isis_eps__reset__to_t cmd_t;
//	isis_eps__reset__from_t cmd_f;
//	cmd_t.fields.rst_key = RESET_KEY;
	isismepsv2_ivid7_piu__replyheader_t reply;
	logError(isismepsv2_ivid7_piu__reset(EPS_I2C_BUS_INDEX, &reply),"CMD_ResetComponent-isis_eps__reset__tmtc");
}

void Maintenance()
{
	SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);

	//logError(IsFS_Corrupted());-> we send corrupted bytes over beacon, no need to log in error file all the time

	// check if for too long we didn't got any comm from ground, and reset TRXVU and sat if needed
	if (IsGroundCommunicationWDTKick()){
		logError(INFO_MSG,"Maintenance-WDTKick, going to restart systems");
		ResetGroundCommWDT(); // to make sure we don't get into endless restart loop
		char PayloadState = 1;//disable payload
		FRAM_write((unsigned char*)&PayloadState,PAYLOAD_IS_DEAD_ADDR,PAYLOAD_IS_DEAD_SIZE);
		// hard reset the TRXVU
		logError(isis_vu_e__reset_hw_tx(ISIS_TRXVU_I2C_BUS_INDEX),"Maintenance-IsisTrxvu_hardReset tx");
		vTaskDelay(1 / portTICK_RATE_MS);
		logError(isis_vu_e__reset_hw_rx(ISIS_TRXVU_I2C_BUS_INDEX),"Maintenance-IsisTrxvu_hardReset rx");
		vTaskDelay(500);
		SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR,MOST_UPDATED_SAT_TIME_SIZE);// store the most updated sat time
		HardResetMCU();
	}

	DeleteOldFiels(MIN_FREE_SPACE);

	CheckDeployAnt();

	Payload_Safety_IN_Maintenance();
}

