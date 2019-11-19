#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/Storage/FRAM.h>
#include <hal/errors.h>

#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/PowerManagment/EPSOperationModes.h"

#include <satellite-subsystems/IsisTRXVU.h>

#include "GlobalStandards.h"
#include "FRAM_FlightParameters.h"
#include "SPL.h"
#include "CommunicationHelper.h"



unsigned char g_current_beacon_period = 0;		// marks the current beacon cycle(how many were transmitted before change in baud)
unsigned char g_beacon_change_baud_period = 0;	// every 'g_beacon_change_baud_period' beacon will be in 1200Bps and not 9600Bps


time_unix g_prev_beacon_time = 0;				// the time at which the previous beacon occured
time_unix g_beacon_interval_time = 0;			// seconds between each beacon

Boolean 		g_mute_flag = MUTE_OFF;				// mute flag - is the mute enabled
time_unix 		g_mute_end_time = 0;				// time at which the mute will end

xSemaphoreHandle xIsTransmitting = NULL; // mutex on transmission.

void InitTxModule()
{
	if(NULL == xIsTransmitting)
		vSemaphoreCreateBinary(xIsTransmitting);
}


void InitBeaconParams()
{
	if(0!= FRAM_read(&g_beacon_change_baud_period,BEACON_BITRATE_CYCLE_ADDR,BEACON_BITRATE_CYCLE_SIZE)){
		g_beacon_change_baud_period = DEFALUT_BEACON_BITRATE_CYCLE;
	}

	if (0	!= FRAM_read((unsigned char*) &g_beacon_interval_time,BEACON_INTERVAL_TIME_ADDR,BEACON_INTERVAL_TIME_SIZE)){
		g_beacon_interval_time = DEFAULT_BEACON_INTERVAL_TIME;
	}
}

