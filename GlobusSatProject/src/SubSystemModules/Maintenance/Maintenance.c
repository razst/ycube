#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>

#include "GlobalStandards.h"

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <hcc/api_fat.h>

#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "TLM_management.h"
#include "Maintenance.h"
#include "utils.h"

Boolean CheckExecutionTime(time_unix prev_time, time_unix period)
{
	time_unix curr = 0;
	if (logError(Time_getUnixEpoch(&curr)) == 1) return -1;
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

	if (reset_flag) {
		time_unix curr_time = 0;
		Time_getUnixEpoch(&curr_time);

		err = SendAckPacket(ACK_RESET_WAKEUP, NULL, (unsigned char*) &curr_time,
				sizeof(time_unix));

		reset_flag = FALSE_8BIT;
		FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);

		FRAM_read((unsigned char*) &num_of_resets,
		NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
		num_of_resets++;

		FRAM_write((unsigned char*) &num_of_resets,
		NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
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
// and return a boolean describing it.
Boolean IsGroundCommunicationWDTKick()
{
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	time_unix last_comm_time = 0;
	FRAM_read((unsigned char*) &last_comm_time, LAST_COMM_TIME_ADDR,
	LAST_COMM_TIME_SIZE);

	time_unix wdt_kick_thresh = GetGsWdtKickTime();

	//TODO: if current_time - last_comm_time < 0
	if (current_time - last_comm_time >= wdt_kick_thresh) {
		return TRUE;
	}
	return FALSE;
}

//TODO: add to command dictionary
int SetGsWdtKickTime(time_unix new_gs_wdt_kick_time)
{
	int err = FRAM_write((unsigned char*)&new_gs_wdt_kick_time, NO_COMM_WDT_KICK_TIME_ADDR,
		NO_COMM_WDT_KICK_TIME_SIZE);
	return err;
}

time_unix GetGsWdtKickTime()
{
	time_unix no_comm_thresh = 0;
	FRAM_read((unsigned char*)&no_comm_thresh, NO_COMM_WDT_KICK_TIME_ADDR,
	NO_COMM_WDT_KICK_TIME_SIZE);
	return no_comm_thresh;
}

void Maintenance()
{
	SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR,
	MOST_UPDATED_SAT_TIME_SIZE);

	//TODO: do error log file
	logError(WakeupFromResetCMD());
	//TODO do someting
	logError(IsFS_Corrupted());

	logError(IsGroundCommunicationWDTKick());
}

