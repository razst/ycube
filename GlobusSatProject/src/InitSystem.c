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
#include "utils.h"

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
	return FALSE;
}

void firstActivationProcedure()
{
}

void WriteDefaultValuesToFRAM()
{
}

int StartFRAM()
{
	if(logError(FRAM_start()))
		return -1;
	else
		return 0;
}

int StartI2C()
{
	if(logError(I2C_start(I2c_SPEED_Hz, I2c_Timeout)))
		return -1;
	else
		return 0;
}

int StartSPI()
{
	if(logError(SPI_start(bus1_spi, slave1_spi)))
		return -1;
	else
		return 0;
}

int StartTIME()
{
	return 0;
}

int DeploySystem()
{
	return 0;
}

#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);
int InitSubsystems()
{
	StartFRAM();
	StartI2C();
	EPS_Init();

	return 0;
}

