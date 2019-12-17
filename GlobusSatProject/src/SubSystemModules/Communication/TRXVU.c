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
#include "CommunicationHelper.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#ifdef TESTING_TRXVU_FRAME_LENGTH
#include <hal/Utility/util.h>
#endif


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
	if(xDumpLock == NULL)
		vSemaphoreCreateBinary(xDumpLock);
	if(xDumpQueue == NULL)
		xDumpQueue = xQueueCreate(1, sizeof(Boolean));
}


int InitTrxvu() {
	ISIStrxvuI2CAddress myTRXVUAddress;
	ISIStrxvuFrameLengths myTRXVUFramesLenght;


	//Buffer definition
	myTRXVUFramesLenght.maxAX25frameLengthTX = SIZE_TXFRAME;//SIZE_TXFRAME;
	myTRXVUFramesLenght.maxAX25frameLengthRX = SIZE_RXFRAME;

	//I2C addresses defined
	myTRXVUAddress.addressVu_rc = I2C_TRXVU_RC_ADDR;
	myTRXVUAddress.addressVu_tc = I2C_TRXVU_TC_ADDR;


	//Bitrate definition
	ISIStrxvuBitrate myTRXVUBitrates;
	myTRXVUBitrates = trxvu_bitrate_9600; // TODO should we use bit rate 1200?? for beacon??
	if (logError(IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUFramesLenght,&myTRXVUBitrates, 1))) return -1;

	vTaskDelay(100); //TODO why 100?? 100 what??

	if (logError(IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,myTRXVUBitrates))) return -1;
	vTaskDelay(100);


	ISISantsI2Caddress myAntennaAddress;
	myAntennaAddress.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	myAntennaAddress.addressSideB = ANTS_I2C_SIDE_B_ADDR;

	//Initialize the AntS system
	if (logError(IsisAntS_initialize(&myAntennaAddress, 1))) return -1;

	InitTxModule();
	InitBeaconParams();
	InitSemaphores();


	return 0;
}

int TRX_Logic() {
	int err = 0;
	// check if we have frames (data) waiting in the TRXVU
	int frameCount = GetNumberOfFramesInBuffer();
	sat_packet_t cmd = { 0 }; // holds the SPL command data

	if (frameCount > 0) {
		// we have data that came from grand station
		err = GetOnlineCommand(&cmd); //--> check - don't reset WDT if we got error getting the frame becuase we will never get a reset !
		ResetGroundCommWDT();
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);

	} else if (GetDelayedCommandBufferCount() > 0) {
		err = GetDelayedCommand(&cmd);
	}




	if (err == command_found) {
		err = ActUponCommand(&cmd);
		//TODO: log error
		//TODO: send message to ground when a delayed command was not executed-> add to log
	}
	BeaconLogic();

	if (logError(err)) return -1;


	return command_succsess;



}

int GetNumberOfFramesInBuffer() {
	unsigned short frameCounter = 0;
	if (logError(IsisTrxvu_rcGetFrameCount(0, &frameCounter))) return -1;

	return frameCounter;
}


int GetOnlineCommand(sat_packet_t *cmd)
{
	if (NULL == cmd) {
		return null_pointer_error;
	}
	int err = 0;

	unsigned short frameCount = 0;
	unsigned char receivedFrameData[MAX_COMMAND_DATA_LENGTH];

	if (logError(IsisTrxvu_rcGetFrameCount(0, &frameCount))) return -1;

	if (0 == frameCount) {
		return no_command_found;
	}

	ISIStrxvuRxFrame rxFrameCmd = { 0, 0, 0,
			(unsigned char*) receivedFrameData }; // for getting raw data from Rx, nullify values

	if (logError(IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd))) return -1;

	if (logError(ParseDataToCommand(receivedFrameData,cmd))) return -1;


	return command_found;

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
