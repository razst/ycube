#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "SubsystemCommands/TRXVU_Commands.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#ifdef TESTING_TRXVU_FRAME_LENGTH
#include <hal/Utility/util.h>
#endif


time_unix 		g_idle_end_time = 1;				// time at which the idel will end

xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xSemaphoreHandle xIsTransmitting = NULL; // mutex on transmission.
xTaskHandle xDumpHandle = NULL;

time_unix g_prev_beacon_time = 0;				// the time at which the previous beacon occured
time_unix g_beacon_interval_time = 0;			// seconds between each beacon



// set mute end time in FRAM
void setMuteEndTime(time_unix endTime){
	logError(FRAM_write((unsigned char*) &endTime , MUTE_END_TIME_ADDR , MUTE_END_TIME_SIZE) ,"TRXVU-setMuteEndTime");
}

// get mute end time from FRAM
time_unix getMuteEndTime(){
	time_unix endTime;
	logError(FRAM_read((unsigned char*) &endTime , MUTE_END_TIME_ADDR , MUTE_END_TIME_SIZE) ,"TRXVU-getMuteEndTime");
	return endTime;
}

// set Transponder end time in FRAM
void setTransponderEndTime(time_unix endTime){
	logError(FRAM_write((unsigned char*) &endTime , TRANSPONDER_END_TIME_ADDR , TRANSPONDER_END_TIME_SIZE) ,"TRXVU-setTransponderEndTime");
}

// get Transponder end time from FRAM
time_unix getTransponderEndTime(){
	time_unix endTime;
	logError(FRAM_read((unsigned char*) &endTime , TRANSPONDER_END_TIME_ADDR , TRANSPONDER_END_TIME_SIZE) ,"TRXVU-getTransponderEndTime");
	return endTime;
}


// set Transponder RSSI in FRAM
void setTransponderRSSIinFRAM(short val){
	logError(FRAM_write((unsigned char*) &val , TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE) ,"TRXVU-setTransponderRSSI");
}

// get Transponder end time from FRAM
short getTransponderRSSIFromFRAM(){
	short val;
	logError(FRAM_read((unsigned char*) &val , TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE) ,"TRXVU-getTransponderRSSI");
	return val;
}



void InitTxModule()
{
	if(xIsTransmitting == NULL)
		vSemaphoreCreateBinary(xIsTransmitting);
}

int CMD_SetBeaconInterval(sat_packet_t *cmd)
{
	int err = 0;
	time_unix interval = 0;

	memcpy(&interval,&cmd->data,sizeof(time_unix));

	if (interval < MIN_BEACON_INTRAVL)
		return BEACON_INTRAVL_TOO_SMALL;

	err =  FRAM_write((unsigned char*) &cmd->data,
			BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	err += FRAM_read((unsigned char*) &interval,
			BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	g_beacon_interval_time=interval;
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &interval,sizeof(interval));
	SendAckPacket(ACK_COMD_EXEC, cmd,NULL,0);
	return err;
}



void InitBeaconParams()
{

	if (0	!= FRAM_read((unsigned char*) &g_beacon_interval_time,BEACON_INTERVAL_TIME_ADDR,BEACON_INTERVAL_TIME_SIZE)){
		g_beacon_interval_time = DEFAULT_BEACON_INTERVAL_TIME;
	}
}

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
	myTRXVUBitrates = trxvu_bitrate_9600;
	if (logError(IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUFramesLenght,&myTRXVUBitrates, 1) ,"InitTrxvu-IsisTrxvu_initialize") ) return -1;

	vTaskDelay(1000); // wait a little

	ISISantsI2Caddress myAntennaAddress;
	myAntennaAddress.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	myAntennaAddress.addressSideB = ANTS_I2C_SIDE_B_ADDR;

	//Initialize the AntS system
	if (logError(IsisAntS_initialize(&myAntennaAddress, 1),"InitTrxvu-IsisAntS_initialize")) return -1;

	InitTxModule();
	InitBeaconParams();
	InitSemaphores();
	checkTransponderStart();// lets see if we need to turn on the transponder


	return 0;
}

void checkIdleFinish(){
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);

	// check if it is time to turn off the idle...
	if (g_idle_end_time !=0 && g_idle_end_time < curr_tick_time){
		g_idle_end_time = 0;
		SetIdleState(trxvu_idle_state_off,0);
	}
}

void checkTransponderFinish(){
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);

	// check if it is time to turn off the transponder...
	if (getTransponderEndTime() != 0 && getTransponderEndTime() < curr_tick_time){
		setTransponderEndTime(0);
		char data[2] = {0, 0};
		data[0] = 0x38;
		data[1] = trxvu_transponder_off;
		I2C_write(I2C_TRXVU_TC_ADDR, data, 2);
		logError(INFO_MSG,"transponder off");
	}
}

void checkTransponderStart(){
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);

	// check if we need to turn on the transponder...
	if (getTransponderEndTime() != 0 && getTransponderEndTime() > curr_tick_time){
		turnOnTransponder();
	}
}


int turnOnTransponder(){
	char data[2] = {0, 0};
	data[0] = 0x38;
	data[1] = trxvu_transponder_on;


	// first set the RSSI from value in FRAM
	SetRSSITransponder(getTransponderRSSIFromFRAM());
	vTaskDelay(100);// make sure RSSI was set
	// than turn on the trasnponder
	if (logError(I2C_write(I2C_TRXVU_TC_ADDR, data, 2),"TRXVU-turnOnTransponder") != E_NO_SS_ERR) return -1;

	logError(INFO_MSG,"transponder on");
	return 0;
}

int SetRSSITransponder(short rssiValue)
{
	//sends I2C command
	char data[1+sizeof(short)] = {0};

	data[0] = 0x52;
	memcpy(data+1,&rssiValue,sizeof(rssiValue));

	return logError(I2C_write(I2C_TRXVU_TC_ADDR, data, sizeof(data)),"TRXVU-SetRSSITransponder");
}

int TRX_Logic() {
	int err = 0;
	int cmdFound=-1;
	// check if we have frames (data) waiting in the TRXVU
	int frameCount = GetNumberOfFramesInBuffer();
	sat_packet_t cmd = { 0 }; // holds the SPL command data

	if (frameCount > 0) {
		// we have data that came from grand station
		cmdFound = GetOnlineCommand(&cmd);
	}

	if (cmdFound == command_found) {
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);
		ResetGroundCommWDT();
		err = ActUponCommand(&cmd);
	}

	checkTransponderFinish();
	checkIdleFinish();
	BeaconLogic(FALSE);

	if (logError(err ,"TRX_Logic-ActUponCommand")) return -1;

	return command_succsess;
}

int GetNumberOfFramesInBuffer() {
	unsigned short frameCounter = 0;
	if (logError(IsisTrxvu_rcGetFrameCount(0, &frameCounter) ,"TRX_Logic-IsisTrxvu_rcGetFrameCount")) return -1;

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

	if (logError(IsisTrxvu_rcGetFrameCount(0, &frameCount) ,"GetOnlineCommand-IsisTrxvu_rcGetFrameCount")) return -1;

	if (frameCount==0) {
		return no_command_found;
	}

	ISIStrxvuRxFrame rxFrameCmd = { 0, 0, 0,
			(unsigned char*) receivedFrameData }; // for getting raw data from Rx, nullify values

	if (logError(IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd) ,"GetOnlineCommand-IsisTrxvu_rcGetCommandFrame")) return -1;

	// log frame info
	char buffer [80];
	sprintf (buffer, "Frame info: doppler: %d length: %d rssi: %d", rxFrameCmd.rx_doppler,rxFrameCmd.rx_length,rxFrameCmd.rx_rssi);
	logError(INFO_MSG ,buffer);

	if (logError(ParseDataToCommand(receivedFrameData,cmd),"GetOnlineCommand-ParseDataToCommand")) return -1;


	return command_found;

}

// check the txSemaphore & low_voltage_flag
Boolean CheckTransmitionAllowed() {
	Boolean low_voltage_flag = TRUE;

	low_voltage_flag = EpsGetLowVoltageFlag();
	if (low_voltage_flag) return FALSE;

	// get current unix time
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);

	if (curr_tick_time < getMuteEndTime()) return FALSE;


	// check that we can take the tx Semaphore
	if(pdTRUE == xSemaphoreTake(xIsTransmitting,WAIT_TIME_SEM_TX)){
		xSemaphoreGive(xIsTransmitting);
		return TRUE;
	}
	return FALSE;

}

void FinishDump(sat_packet_t *cmd,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {

	SendAckPacket(acktype, cmd, err, size);
/*
	if (NULL != task_args) {
		free(task_args);
	}*/
	if (NULL != xDumpLock) {
		xSemaphoreGive(xDumpLock);
	}

	if (NULL != xIsTransmitting) {
		xSemaphoreGive(xIsTransmitting);
	}

	//logError(f_releaseFS() ,"FinishDump-f_releaseFS");
	f_releaseFS();

	if (xDumpHandle != NULL) {
		xDumpHandle = NULL;
		vTaskDelete(xDumpHandle);
	}
	if(NULL != buffer){
		free(buffer);
	}
}


void AbortDump(sat_packet_t *cmd)
{
	FinishDump(cmd,NULL,ACK_DUMP_ABORT,NULL,0);
}

void SendDumpAbortRequest() {
	//if (eTaskGetState(xDumpHandle) == eDeleted) {
	if (xDumpHandle == NULL) {
		return;
	}
	Boolean queue_msg = TRUE;
	logError(xQueueSend(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(5)) ,"AbortDump-xQueueSend");
}

Boolean CheckDumpAbort() {
	portBASE_TYPE queueError;
	Boolean queue_msg = FALSE;
	queueError = xQueueReceive(xDumpQueue, &queue_msg, 0);
	if (queueError == pdTRUE)
	{
		if (queue_msg == TRUE)
		{
			return TRUE;
		}
	}

	return FALSE;
}




int BeaconLogic(Boolean forceTX) {

	if(!forceTX && !CheckTransmitionAllowed()){
		return E_CANT_TRANSMIT;
	}

	int err = 0;
	if (!forceTX && !CheckExecutionTime(g_prev_beacon_time, g_beacon_interval_time)) {
		return E_TOO_EARLY_4_BEACON;
	}

	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry(&wod);


	sat_packet_t cmd = { 0 };

	if (logError(AssembleCommand((unsigned char*) &wod, sizeof(wod), trxvu_cmd_type,BEACON_SUBTYPE, BEACON_SPL_ID, &cmd), "BeaconLogic-AssembleCommand") ) return -1;

	// set the current time as the previous beacon time
	if (logError(Time_getUnixEpoch(&g_prev_beacon_time),"BeaconLogic-Time_getUnixEpoch") ) return -1;


	if (logError(TransmitSplPacket(&cmd, NULL),"BeaconLogic-TransmitSplPacket") ) return -1;
	// make sure we switch back to 9600 if we used 1200 in the beacon
	if (logError(IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600),"BeaconLogic-IsisTrxvu_tcSetAx25Bitrate") ) return -1;

	//printf("***************** beacon sent *****************\n");
	return 0;
}


int SetIdleState(ISIStrxvuIdleState state, time_unix duration){

	if (duration > MAX_IDLE_TIME) {
		logError(TRXVU_IDLE_TOO_LONG ,"SetIdleState");
		return TRXVU_IDLE_TOO_LONG;
	}

	// get current unix time
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);
	if (state == trxvu_idle_state_on && curr_tick_time < getMuteEndTime()) return TRXVU_IDEL_WHILE_MUTE;

	if (state == trxvu_idle_state_on && curr_tick_time < getTransponderEndTime()) return TRXVU_IDLE_WHILE_TRANSPONDER;

	int err = logError(IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX, state) ,"SetIdleState-IsisTrxvu_tcSetIdlestate");

	if (err == E_NO_SS_ERR && state == trxvu_idle_state_on){
		logError(INFO_MSG,"Idel ON\n");
		// set idle end time
		g_idle_end_time = curr_tick_time + duration;
	} else if (err == E_NO_SS_ERR && state == trxvu_idle_state_off){
		logError(INFO_MSG,"Idel OFF\n");
	}
	return err;

}


int muteTRXVU(time_unix duration) {
	if (duration > MAX_MUTE_TIME) {
		logError(TRXVU_MUTE_TOO_LONG ,"muteTRXVU");
		return TRXVU_MUTE_TOO_LONG;
	}
	// get current unix time
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);

	// set mute end time
	setMuteEndTime(curr_tick_time + duration);
	return 0;

}

void UnMuteTRXVU() {
	setMuteEndTime(0);

}


int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data,
		unsigned short length) {
	int err = 0;
	sat_packet_t packet = { 0 };
	if (NULL != cmd) {
		err = AssembleCommand(data, length, cmd->cmd_type, cmd->cmd_subtype,
				cmd->ID, &packet);
	} else {
		err = AssembleCommand(data, length, 0xFF, 0xFF, 0xFFFFFFFF, &packet);
	}
	if (err != 0) {
		return err;
	}
	err = TransmitSplPacket(&packet, NULL);
	return err;

}

int TransmitSplPacket(sat_packet_t *packet, int *avalFrames) {
	if (!CheckTransmitionAllowed()) {
		return E_CANT_TRANSMIT;
	}

	if ( packet == NULL) {
		return E_NOT_INITIALIZED;
	}

	int err = 0;
	unsigned short data_length = packet->length + sizeof(packet->length)
			+ sizeof(packet->cmd_subtype) + sizeof(packet->cmd_type)
			+ sizeof(packet->ID);

	if (xSemaphoreTake(xIsTransmitting,SECONDS_TO_TICKS(WAIT_TIME_SEM_TX)) != pdTRUE) {
		return E_GET_SEMAPHORE_FAILED;
	}

	int avail=0;
	err = IsisTrxvu_tcSendAX25DefClSign(ISIS_TRXVU_I2C_BUS_INDEX,
			(unsigned char*) packet, data_length, &avail);

	////printf("avial TRXVU:%d\n",avail);

	if (avail < MIN_TRXVU_BUFF){
		vTaskDelay(100);
	}


	if (err != E_NO_SS_ERR){
		logError(err ,"TRXVU-TransmitSplPacket");
	}

#ifdef TESTING
	printf("trxvu send ax25 error= %d",err);
	 display cmd packet values to screen
	printf(" id= %d",packet->ID);
	printf(" cmd type= %d",packet->cmd_type);
	printf(" cmd sub type= %d",packet->cmd_subtype);
	printf(" length= %d\n",packet->length);
	//printf(" data= %s\n",packet->data);
#endif

	xSemaphoreGive(xIsTransmitting);

	return err;

}

