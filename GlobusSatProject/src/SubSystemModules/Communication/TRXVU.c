#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>

#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#ifdef TESTING_TRXVU_FRAME_LENGTH
#include <hal/Utility/util.h>
#endif
#define SIZE_RXFRAME	200
#define SIZE_TXFRAME	235


Boolean 		g_mute_flag = MUTE_OFF;				// mute flag - is the mute enabled
time_unix 		g_mute_end_time = 0;				// time at which the mute will end
time_unix 		g_prev_beacon_time = 0;				// the time at which the previous beacon occured
time_unix 		g_beacon_interval_time = 0;			// seconds between each beacon
unsigned char	g_current_beacon_period = 0;		// marks the current beacon cycle(how many were transmitted before change in baud)
unsigned char 	g_beacon_change_baud_period = 0;	// every 'g_beacon_change_baud_period' beacon will be in 1200Bps and not 9600Bps

xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task
xSemaphoreHandle xIsTransmitting = NULL; // mutex on transmission.

void InitSemaphores()
{
}

int InitTrxvu() {
	return 0;
}

int TRX_Logic() {
	return 0;
}

int GetNumberOfFramesInBuffer() {
	return 0;
}

Boolean CheckTransmitionAllowed() {
	return FALSE;
}


void FinishDump(dump_arguments_t *task_args,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {
}

void AbortDump()
{
}

void SendDumpAbortRequest() {
}

Boolean CheckDumpAbort() {
	return FALSE;
}

void DumpTask(void *args) {
}

int DumpTelemetry(sat_packet_t *cmd) {
	return 0;
}

//Sets the bitrate to 1200 every third beacon and to 9600 otherwise
int BeaconSetBitrate() {
	return 0;
}

void BeaconLogic() {
}

int muteTRXVU(time_unix duration) {
	return 0;
}

void UnMuteTRXVU() {
}

Boolean GetMuteFlag() {
	return FALSE;
}

Boolean CheckForMuteEnd() {
	return FALSE;
}

int GetTrxvuBitrate(ISIStrxvuBitrateStatus *bitrate) {
	return 0;
}

int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data,
		unsigned int length) {
	return 0;
}

int TransmitSplPacket(sat_packet_t *packet, int *avalFrames) {
	return 0;
}

int UpdateBeaconBaudCycle(unsigned char cycle)
{
	return 0;
}

int UpdateBeaconInterval(time_unix intrvl) {
	return 0;
}
