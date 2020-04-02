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
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "InitSystem.h"
#include "TLM_management.h"
#include <satellite-subsystems/IsisAntS.h>
#include <SubSystemModules/Housekepping/TelemetryCollector.h>

#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#define I2c_SPEED_Hz 100000
#define I2c_Timeout 10
#define I2c_TimeoutTest portMAX_DELAY

Boolean isFirstActivation()
{
	Boolean flag = FALSE;
	FRAM_read((unsigned char*) &flag, FIRST_ACTIVATION_FLAG_ADDR,
	FIRST_ACTIVATION_FLAG_SIZE);
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

	time_unix beacon_interval = 0;
	beacon_interval = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_write((unsigned char*) &beacon_interval, BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	// set the reset counter to zero
	unsigned int num_of_resets = 0;
	FRAM_write((unsigned char*) &num_of_resets,
	NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);

	FRAM_write((unsigned char*) &num_of_resets,
	NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);


}

int StartFRAM()
{
	return logError(FRAM_start());
}

int StartI2C()
{
	return logError(I2C_start(I2c_SPEED_Hz , I2c_Timeout));
}

int StartSPI()
{
	return logError(SPI_start(bus1_spi , slave1_spi));
}

int StartTIME()
{
	int error = 0;
		Time expected_deploy_time = UNIX_DATE_JAN_D1_Y2000;
		error = Time_start(&expected_deploy_time, 0);
		if (0 != error) {
			return logError(error);
		}
		time_unix time_before_wakeup = 0;
		if (!isFirstActivation()) {
			FRAM_read((unsigned char*) &time_before_wakeup,
			MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);

			Time_setUnixEpoch(time_before_wakeup);
		}
		printf("********** size of time unix: %d\n",sizeof(time_unix));
		printf("********** size of time int: %d\n",sizeof(int));
		printf("********** size of time long: %d\n",sizeof(long));
		printf("********** size of time unsigned long int : %d\n",sizeof(unsigned long int));
		printf("********** size of time unsigned long int : %d\n",sizeof(unsigned short));

		return 0;
}

int DeploySystem()
{
	Boolean first_activation = isFirstActivation();

	// if this is not a first activation, than nothing to do here... return
	if (!first_activation) return 0;



	// if we are here...it means we are in the first activatio, wait 30min, deploy and make firstActivation flag=false
	int err = 0;

	time_unix seconds_since_deploy = 0;
	err = logError(FRAM_read((unsigned char*) seconds_since_deploy , SECONDS_SINCE_DEPLOY_ADDR , SECONDS_SINCE_DEPLOY_SIZE));
	if (0 != err) {
		seconds_since_deploy = MINUTES_TO_SECONDS(1);	// RBF to 30 min
	}

	// wait 30 min + log telm
	while (seconds_since_deploy < MINUTES_TO_SECONDS(1)) { // RBF to 30 min
		vTaskDelay(SECONDS_TO_TICKS(10));

		FRAM_write((unsigned char*)&seconds_since_deploy, SECONDS_SINCE_DEPLOY_ADDR,
				SECONDS_SINCE_DEPLOY_SIZE);
		if (0 != err) {
			break;
		}
		TelemetryCollectorLogic();

		seconds_since_deploy += 10;

		isis_eps__watchdog__from_t eps_cmd;
		isis_eps__watchdog__tm(EPS_I2C_BUS_INDEX, &eps_cmd);

	}

	// open ants !
	//IsisAntS_autoDeployment(0, isisants_sideA, 10); // TODO: RBF
	//IsisAntS_autoDeployment(0, isisants_sideB, 10);// TODO: RBF

	// set deploy time in FRAM
	time_unix deploy_time = 0;
	Time_getUnixEpoch(&deploy_time);
	FRAM_write((unsigned char*) deploy_time, DEPLOYMENT_TIME_ADDR,
	DEPLOYMENT_TIME_SIZE);

	// set first activation false in FRAM
	first_activation = FALSE;
	FRAM_write((unsigned char*) &first_activation,
	FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);

	// write default values to FRAM
	WriteDefaultValuesToFRAM();
	return 0;
}


#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);
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

	return 0;
}

