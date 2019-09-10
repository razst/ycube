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
}

