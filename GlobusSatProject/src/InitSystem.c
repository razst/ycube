#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>
#include <at91/utility/exithandler.h>
#include <string.h>
#include "GlobalStandards.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/HashSecuredCMD.h"
#include "SubSystemModules/Communication/SubsystemCommands/TRXVU_Commands.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/RAMTelemetry.h"
#include "InitSystem.h"
#include "TLM_management.h"
#include <satellite-subsystems/isis_ants.h>
#include <SubSystemModules/Housekepping/TelemetryCollector.h>

#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
#ifdef ISISEPS
	#include <satellite-subsystems/isismepsv2_ivid7_piu.h>
#endif
#define I2c_SPEED_Hz 100000
#define I2c_Timeout 10
#define I2c_TimeoutTest portMAX_DELAY

Boolean isFirstActivation()
{
	Boolean flag = FALSE;
	FRAM_read((unsigned char*) &flag, FIRST_ACTIVATION_FLAG_ADDR,FIRST_ACTIVATION_FLAG_SIZE);
	return flag;
}


void WriteDefaultValuesToFRAM()
{
	time_unix default_no_comm_thresh;
	default_no_comm_thresh = DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*) &default_no_comm_thresh , NO_COMM_WDT_KICK_TIME_ADDR , NO_COMM_WDT_KICK_TIME_SIZE);

	EpsThreshVolt_t def_thresh_volt = { .raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
		FRAM_write((unsigned char*)def_thresh_volt.raw, EPS_THRESH_VOLTAGES_ADDR,
		EPS_THRESH_VOLTAGES_SIZE);

	float def_alpha;
	def_alpha = DEFAULT_ALPHA_VALUE;
	FRAM_write((unsigned char*) &def_alpha ,EPS_ALPHA_FILTER_VALUE_ADDR , EPS_ALPHA_FILTER_VALUE_SIZE);

	time_unix tlm_save_period = DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, EPS_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, TRXVU_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, ANT_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, SOLAR_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, WOD_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_RADFET_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, RADFET_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	tlm_save_period = DEFAULT_PAYLOAD_EVENTS_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tlm_save_period, PAYLOAD_EVENTS_SAVE_TLM_PERIOD_ADDR,
			sizeof(tlm_save_period));

	time_unix beacon_interval = 0;
	beacon_interval = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_write((unsigned char*) &beacon_interval, BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	short rssi;
	rssi = DEFAULT_RSSI_VALUE;
	FRAM_write((unsigned char*) &rssi ,TRANSPONDER_RSSI_ADDR , TRANSPONDER_RSSI_SIZE);

	// set the reset counter to zero
	unsigned int num_of_resets = 0;
	FRAM_write((unsigned char*) &num_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);

	FRAM_write((unsigned char*) &num_of_resets,
	NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);

	FRAM_write((unsigned char*) &num_of_resets,
	TRANSPONDER_END_TIME_ADDR, TRANSPONDER_END_TIME_SIZE);

	FRAM_write((unsigned char*) &num_of_resets,
	DEL_OLD_FILES_NUM_DAYS_ADDR, DEL_OLD_FILES_NUM_DAYS_SIZE);

	FRAM_write((unsigned char*) &num_of_resets,
	NUMBER_OF_SW3_RESETS_ADDR, NUMBER_OF_SW3_RESETS_SIZE);

	char temp = SD_CARD_DRIVER_PRI;
	FRAM_write((unsigned char*) &temp, ACTIVE_SD_ADDR, ACTIVE_SD_SIZE);

	unsigned int IDtemp = If_ID_is_Empty; // the LAST SPL id - used for secure CMD / HASH functions
	FRAM_write((unsigned char*)&IDtemp, CMD_ID_ADDR, CMD_ID_SIZE);
	
	unsigned int password = SECURED_CMD_PASS;
	FRAM_write((unsigned char*)&password, CMD_PASSWORD_ADDR, CMD_PASSWORD_SIZE);

	Boolean flag = FALSE;
	FRAM_write((unsigned char*) &flag,
			STOP_REDEPOLOY_FLAG_ADDR, STOP_REDEPOLOY_FLAG_SIZE);
	char resetFlag;
	resetFlag = 0;
	FRAM_write((unsigned char*)&resetFlag,HAS_SAT_RESET_ADDR,HAS_SAT_RESET_SIZE);
//	FRAM_write((unsigned char*)&resetFlag,PAYLOAD_IS_DEAD_ADDR,PAYLOAD_IS_DEAD_SIZE);


	ResetGroundCommWDT();

	setMuteEndTime(0);
}

int StartFRAM()
{
	int err = logError(FRAM_start() ,"StartFRAM");
	if (err != E_NO_SS_ERR) return err;
	Boolean first_activation = isFirstActivation();
	if (first_activation){
		WriteDefaultValuesToFRAM();
	}
	return E_NO_SS_ERR;
}

int StartI2C()
{
	return logError(I2C_start(I2c_SPEED_Hz , I2c_Timeout) ,"StartI2C");
}

int StartSPI()
{
	return logError(SPI_start(bus1_spi , slave1_spi) ,"SPI_start");
}

int StartTIME()
{
	int error = 0;
	Time expected_deploy_time = UNIX_DATE_JAN_D1_Y2000;
	error = Time_start(&expected_deploy_time, 0);
	if (0 != error) {
		return logError(error ,"StartTIME-Time_start");
	}
	// udpate to sat time that we had before the restart in FRAM
	time_unix time_before_wakeup = 0;
	if (!isFirstActivation()) {
		FRAM_read((unsigned char*) &time_before_wakeup,
		MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);

		Time_setUnixEpoch(time_before_wakeup + RESTART_TIME); // set the last time we had + time it takes to restart the sat
	}

	return 0;
}



//TODO: before sent to flight: 1. set FIRST_ACTIVATION flag to TRUE 2. set SECONDS_SINCE_DEPLOY to 0
int DeploySystem()
{
	Boolean first_activation = isFirstActivation();

	// if this is not a first activation, than nothing to do here... return
	if (!first_activation) return 0;

	logError(INFO_MSG,"Deploy first activation");
	// write default values to FRAM
	WriteDefaultValuesToFRAM();

	// if we are here...it means we are in the first activation, wait 30min, deploy and make firstActivation flag=false
	int err = 0;

	time_unix seconds_since_deploy = 0;
	err = logError(FRAM_read((unsigned char*) &seconds_since_deploy , SECONDS_SINCE_DEPLOY_ADDR , SECONDS_SINCE_DEPLOY_SIZE) ,"DeploySystem-FRAM_read");
	if (err != E_NO_SS_ERR) {
		seconds_since_deploy = 0;
	}

	time_unix startTime = 0;
	Time_getUnixEpoch(&startTime);
	startTime -= seconds_since_deploy;

	// wait 30 min + log telm
	while (seconds_since_deploy < MINUTES_TO_SECONDS(MIN_2_WAIT_BEFORE_DEPLOY)) {
		logError(INFO_MSG,"Deploy wait loop start");
		printf("deploy loop, sec since deploy=%d \n\r",seconds_since_deploy);
		// wait 10 sec and update timer in FRAM
		vTaskDelay(SECONDS_TO_TICKS(10));

		time_unix currTime = 0;
		Time_getUnixEpoch(&currTime);
		seconds_since_deploy = currTime - startTime;

		logError(FRAM_write((unsigned char*)&seconds_since_deploy, SECONDS_SINCE_DEPLOY_ADDR,
				SECONDS_SINCE_DEPLOY_SIZE),"DeploySystem-FRAM_write");

		// collect TLM
		TelemetryCollectorLogic();

		// reset WDT
		isismepsv2_ivid7_piu__replyheader_t res;
		isismepsv2_ivid7_piu__resetwatchdog(EPS_I2C_BUS_INDEX,&res);

	}
	logError(INFO_MSG,"Deploy wait loop - DONE");

	// open ants !
	CMD_AntennaDeploy(NULL);

	// set deploy time in FRAM
	time_unix deploy_time = 0;
	Time_getUnixEpoch(&deploy_time);
	FRAM_write((unsigned char*) &deploy_time, DEPLOYMENT_TIME_ADDR,
			DEPLOYMENT_TIME_SIZE);

	// set first activation false in FRAM
	first_activation = FALSE;
	FRAM_write((unsigned char*) &first_activation,
			FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);

	return 0;
}


int InitSubsystems()
{
	StartI2C();

	StartSPI();

	StartFRAM();

	StartTIME();

	InitializeFS(isFirstActivation());

	InitSavePeriodTimes();

	EPS_Init();

	InitTrxvu();

	DeploySystem();

	WakeupFromResetCMD();

	ResetRamTlm();

	logError(INFO_MSG ,"Sat Started");

	vTaskDelay(1000); // rest a little before we start working

	return 0;
}

