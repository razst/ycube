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
	return 0;
}

int StartI2C()
{
	return 0;
}

int StartSPI()
{
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
	return 0;
}

