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

#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
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

void firstActivationProcedure()
{
#ifdef ISISEPS
	ieps_statcmd_t eps_cmd;
#endif

	int err = 0;

	time_unix seconds_since_deploy = 0;
	err = FRAM_read((unsigned char*) seconds_since_deploy , SECONDS_SINCE_DEPLOY_ADDR , SECONDS_SINCE_DEPLOY_SIZE);
	if (0 != err) {
		seconds_since_deploy = MINUTES_TO_SECONDS(30);	// deploy immediately. No mercy
	}

	while (seconds_since_deploy < MINUTES_TO_SECONDS(30)) {
		vTaskDelay(SECONDS_TO_TICKS(10));

		FRAM_write((unsigned char*)&seconds_since_deploy, SECONDS_SINCE_DEPLOY_ADDR,
				SECONDS_SINCE_DEPLOY_SIZE);
		if (0 != err) {
			break;
		}
		TelemetryCollectorLogic();

		seconds_since_deploy += 10;

		IsisEPS_resetWDT(EPS_I2C_BUS_INDEX, &eps_cmd);

	}

#ifndef TESTING
	IsisAntS_autoDeployment(0, isisants_sideA, 10);
	IsisAntS_autoDeployment(0, isisants_sideB, 10);
#endif
	//TODO: log
}

void WriteDefaultValuesToFRAM()
{
	time_unix default_no_comm_thresh;
	default_no_comm_thresh = DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*) &default_no_comm_thresh , NO_COMM_WDT_KICK_TIME_ADDR , NO_COMM_WDT_KICK_TIME_SIZE);

	voltage_t def_thresg_volt;
	def_thresg_volt = { .raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
	FRAM_write((unsigned char*) &def_thresg_volt ,EPS_THRESH_VOLTAGES_ADDR , EPS_THRESH_VOLTAGES_SIZE);

	float def_alpha;
	def_alpha = DEFAULT_ALPHA_VALUE;
	FRAM_write((unsigned char*) &def_alpha ,EPS_ALPHA_FILTER_VALUE_ADDR , EPS_ALPHA_FILTER_VALUE_SIZE);

	time_unix tim_save_period;
	tim_save_period = DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write((unsigned char*) &tim_save_period , DEFAULT_WOD_SAVE_TLM_TIME , sizeof(tim_save_period));

}

int StartFRAM()
{
	return FRAM_start();
}

int StartI2C()
{
	return I2C_start(I2c_SPEED_Hz , I2c_Timeout);
}

int StartSPI()
{
	return SPI_start(bus1_spi , slave1_spi);
}

int StartTIME()
{
	int error = 0;
		Time expected_deploy_time = UNIX_DATE_JAN_D1_Y2000;
		error = Time_start(&expected_deploy_time, 0);
		if (0 != error) {
			return error;
		}
		time_unix time_before_wakeup = 0;
		if (!isFirstActivation()) {
			FRAM_read((unsigned char*) &time_before_wakeup,
			MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);

			Time_setUnixEpoch(time_before_wakeup);
		}
		return 0;
}

int DeploySystem()
{
	Boolean first_activation = isFirstActivation();

		if (first_activation) {

			firstActivationProcedure();

			time_unix deploy_time = 0;
			Time_getUnixEpoch(&deploy_time);
			FRAM_write((unsigned char*) deploy_time, DEPLOYMENT_TIME_ADDR,
			DEPLOYMENT_TIME_SIZE);


			first_activation = FALSE;
			FRAM_write((unsigned char*) &first_activation,
			FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);

			WriteDefaultValuesToFRAM();
		}
		return 0;
}

#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);
int InitSubsystems()
{
	EPS_Init();

	return 0;
}

