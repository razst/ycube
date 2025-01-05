#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/isis_vu_e.h>
#include <satellite-subsystems/isis_ants.h>
#include <satellite-subsystems/isis_ants_types.h>
#include <satellite-subsystems/isismepsv2_ivid7_piu.h>
#include <satellite-subsystems/isismepsv2_ivid7_piu_types.h>


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

#include "HashSecuredCMD.h"


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

	// *** init TRXVU ***
    // Definition of I2C and TRXUV
    ISIS_VU_E_t myTRXVU[1];

	//I2C addresses defined
    myTRXVU[0].rxAddr = I2C_TRXVU_RC_ADDR;
    myTRXVU[0].txAddr = I2C_TRXVU_TC_ADDR;

	//Buffer definition
    myTRXVU[0].maxSendBufferLength = MAX_COMMAND_DATA_LENGTH;
    myTRXVU[0].maxReceiveBufferLength = MAX_COMMAND_DATA_LENGTH;

	if (logError(ISIS_VU_E_Init(myTRXVU, 1) ,"InitTrxvu-IsisTrxvu_initialize") ) return -1;
	
	if(ChangeTrxvuConfigValues()){return -1;}

	vTaskDelay(1000); // wait a little

	// *** init ANts ****
	// turn on 5V SW2 for ANTs
	uint8_t index = 0;
	isismepsv2_ivid7_piu__replyheader_t response;
	logError(isismepsv2_ivid7_piu__outputbuschannelon(index, isismepsv2_ivid7_piu__imeps_channel__channel_5v_sw2, &response), "Turn on ANT channel");
	vTaskDelay(100); // wait a little

    ISIS_ANTS_t myAntennaAddress[2];
    //Primary
	myAntennaAddress[0].i2cAddr = ANTS_I2C_SIDE_A_ADDR;
	//Secondary
	myAntennaAddress[1].i2cAddr = ANTS_I2C_SIDE_B_ADDR;

	int err = ISIS_ANTS_Init(myAntennaAddress, 2);
	if (logError(err,"InitTrxvu-IsisAntS_initialize")) return -1;

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
		SetIdleState(isis_vu_e__onoff__off,0);
	}
}

void checkTransponderFinish(){
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);

	// check if it is time to turn off the transponder...
	if (getTransponderEndTime() != 0 && getTransponderEndTime() < curr_tick_time){
		turnOffTransponder();
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
	// then turn on the transponder
	if (logError(I2C_write(I2C_TRXVU_TC_ADDR, data, 2),"TRXVU-turnOnTransponder") != E_NO_SS_ERR) return -1;

	logError(INFO_MSG,"transponder on");
	return E_NO_SS_ERR;
}


int turnOffTransponder(){
	setTransponderEndTime(0);
	char data[2] = {0, 0};
	data[0] = 0x38;
	data[1] = trxvu_transponder_off;
	if (logError(I2C_write(I2C_TRXVU_TC_ADDR, data, 2),"TRXVU-turnOffTransponder") != E_NO_SS_ERR) return -1;
	logError(INFO_MSG,"transponder off");
	return E_NO_SS_ERR;
}

int SetRSSITransponder(short rssiValue)
{
	//sends I2C command
	char data[1 + sizeof(short)] = {0};

	data[0] = 0x52;
	memcpy(data + 1,&rssiValue,sizeof(rssiValue));

	return logError(I2C_write(I2C_TRXVU_TC_ADDR, data, sizeof(data)),"TRXVU-SetRSSITransponder");
}

int TRX_Logic() {
	int err = 0;
	int cmdFound = -1;
	// check if we have frames (data) waiting in the TRXVU
	int frameCount = GetNumberOfFramesInBuffer();
	sat_packet_t cmd = { 0 }; // holds the SPL command data

	if (frameCount > 0) {
		// we have data that came from ground station
		printf("Got new frame ...%d\r\n",frameCount);
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
	unsigned int timeoutCounter = 0;

	while(timeoutCounter < 4*TIMEOUT_UPBOUND && frameCounter==0)
	{
		if (logError(isis_vu_e__get_frame_count(0, &frameCounter) ,"TRX_Logic-IsisTrxvu_rcGetFrameCount")) return -1;

		timeoutCounter++;

		vTaskDelay(10 / portTICK_RATE_MS);
	}

	return frameCounter;
}


int GetOnlineCommand(sat_packet_t *cmd)
{
	if (NULL == cmd) {
		return null_pointer_error;
	}
	int err = 0;

	uint16_t frameCount = 0;
	unsigned char receivedFrameData[MAX_COMMAND_DATA_LENGTH];

	if (logError(isis_vu_e__get_frame_count(0, &frameCount) ,"GetOnlineCommand-IsisTrxvu_rcGetFrameCount")) return -1;

	if (frameCount==0) {
		return no_command_found;
	}

	isis_vu_e__get_frame__from_t rxFrameCmd = { 0, 0, 0,
			(unsigned char*) receivedFrameData }; // for getting raw data from Rx, nullify values

	if (logError(isis_vu_e__get_frame(0, &rxFrameCmd) ,"GetOnlineCommand-IsisTrxvu_rcGetCommandFrame")) return -1;

	// log frame info
	char buffer [80];
	sprintf (buffer, "Frame info: doppler: %d length: %d rssi: %d", rxFrameCmd.doppler ,rxFrameCmd.length,rxFrameCmd.rssi);
	logError(INFO_MSG ,buffer);
	// remove the last frame
	vTaskDelay(10 / portTICK_RATE_MS);
	logError(isis_vu_e__remove_frame(0),"isis_vu_e__remove_frame");
	vTaskDelay(10 / portTICK_RATE_MS);

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
	
	int err = 0;
	//should be or???
	if(!forceTX && !CheckTransmitionAllowed()){
		return E_CANT_TRANSMIT;
	}

	if (!forceTX && !CheckExecutionTime(g_prev_beacon_time, g_beacon_interval_time)) {
		return E_TOO_EARLY_4_BEACON;
	}

	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry(&wod);
	sat_packet_t cmd = { 0 };


	if (logError(AssembleCommand((unsigned char*) &wod, sizeof(wod), trxvu_cmd_type,BEACON_SUBTYPE, BEACON_SPL_ID, &cmd), "BeaconLogic-AssembleCommand") ) return -1;
	// set the current time as the previous beacon time
	if (logError(Time_getUnixEpoch(&g_prev_beacon_time),"BeaconLogic-Time_getUnixEpoch") ) return -1;

	// update TX config as it gets override after WDT of the TX
	if(ChangeTrxvuConfigValues()){return -1;}

	if (logError(TransmitSplPacket(&cmd, NULL),"BeaconLogic-TransmitSplPacket") ) return -1;

	//printf("***************** beacon sent *****************\n");
	return 0;
}


int SetIdleState(isis_vu_e__onoff_t state, time_unix duration){

	if (duration > MAX_IDLE_TIME) {
		logError(TRXVU_IDLE_TOO_LONG ,"SetIdleState");
		return TRXVU_IDLE_TOO_LONG;
	}

	// get current unix time
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch(&curr_tick_time);
	if (state == isis_vu_e__onoff__on && curr_tick_time < getMuteEndTime()) return TRXVU_IDEL_WHILE_MUTE;

	if (state == isis_vu_e__onoff__on && curr_tick_time < getTransponderEndTime()) return TRXVU_IDLE_WHILE_TRANSPONDER;

	int err = logError(isis_vu_e__set_idle_state(ISIS_TRXVU_I2C_BUS_INDEX, state) ,"SetIdleState-IsisTrxvu_tcSetIdlestate");

	if (err == E_NO_SS_ERR && state == isis_vu_e__onoff__on){
		logError(INFO_MSG,"Idel ON\n");
		// set idle end time
		g_idle_end_time = curr_tick_time + duration;
	} else if (err == E_NO_SS_ERR && state == isis_vu_e__onoff__off){
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
void Hash256(char* text, BYTE* outputHash)
{
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;

    // Initialize SHA256 context
    sha256_init(&ctx);

    // Hash the user input (text)
    sha256_update(&ctx, (BYTE*)text, strlen(text));
    sha256_final(&ctx, buf);

    // Copy the hash into the provided output buffer
    memcpy(outputHash, buf, SHA256_BLOCK_SIZE);
}
char error_hash[8] = {0};
int CMD_Hash256(sat_packet_t *cmd)
{
	unsigned int lastid, currId, code;
    char plsHashMe[50];
    char code_to_str[50];
    char cmpHash[Max_Hash_size], temp[Max_Hash_size];
	
    currId = cmd->ID;

	if (cmd == NULL || cmd->data == NULL) {
		return E_INPUT_POINTER_NULL;
	}

    //get code from FRAM
    FRAM_read((unsigned char*)&code, CMD_PASSWORD_ADDR, CMD_PASSWORD_SIZE);

    //get the last id from FRAM and save it into var lastid then add new id to the FRAM (as new lastid)
    FRAM_read((unsigned char*)&lastid, CMD_ID_ADDR, CMD_ID_SIZE);
    FRAM_write((unsigned char*)&currId, CMD_ID_ADDR, CMD_ID_SIZE);

    //check if curr ID is bigger than lastid
    if(currId <= lastid)
    {
        return E_UNAUTHORIZED;//bc bool FALSE needed?
    }

    //combine lastid(as str) into plshashme
    sprintf(plsHashMe, "%u", currId);

    // turn code into str
    sprintf(code_to_str, "%u", code);

    //add (passcode)
    strcat(plsHashMe, code_to_str);

    // Initialize buffer for hashed output
    BYTE hashed[SHA256_BLOCK_SIZE];

    // Hash the combined string
    Hash256(plsHashMe, hashed);

    //cpy byte by byte to temp (size of otherhashed = 8 bytes *2 (all bytes are saved by twos(bc its in hex))+1 for null)
    char otherhashed[Max_Hash_size * 2 + 1]; // Array to store 8 bytes in hex, plus a null terminator

    /*for (int i = 0; i < Max_Hash_size; i++) {
        sprintf(&otherhashed[i * 2], "%02x", hashed[i]);
    }
    otherhashed[16] = '\0'; // Add Null*/

    //cpy first 8 bytes to temp 
    memcpy(temp, hashed, Max_Hash_size);

	//add temp to globle var 
	memcpy(error_hash, temp, Max_Hash_size);
    //cpy first 8 bytes of the data
    memcpy(cmpHash, cmd -> data, Max_Hash_size);

	if(cmd -> length < Max_Hash_size)
		return E_MEM_ALLOC;

	//fix cmd.data
	cmd -> length = cmd -> length - Max_Hash_size;//8 bytes are removed from the data this must be reflected in the length
	//note: the memove is crucial to the command though can be changed to memcpy this is so the command that comes after can use the cmd. data correctly (taken out bc of error) 
	//memmove(cmd->data, cmd->data + Max_Hash_size,cmd->length - Max_Hash_size);
	
    //cmp hash from command centre to internal hash
    if(memcmp(temp, cmpHash, Max_Hash_size) == 0)
    {   
        printf("success!\n");//for test
		SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
        return E_NO_SS_ERR;
    }
    else
	{
		return E_UNAUTHORIZED;
	}
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


	uint8_t avail=0;
//	err = isis_vu_e__set_bitrate(0, isis_vu_e__bitrate__9600bps);
	err = isis_vu_e__send_frame(ISIS_TRXVU_I2C_BUS_INDEX,(unsigned char*) packet, data_length, &avail);

	////printf("avial TRXVU:%d\n",avail);

	if (avail < MIN_TRXVU_BUFF){
		vTaskDelay(100);
	}


	if (err != E_NO_SS_ERR){
		logError(err ,"TRXVU-TransmitSplPacket");
	}

#ifdef TESTING
	printf("trxvu send ax25 error= %d",err);
//	 display cmd packet values to screen
	printf(" id= %d",packet->ID);
	printf(" cmd type= %d",packet->cmd_type);
	printf(" cmd sub type= %d",packet->cmd_subtype);
	printf(" length= %d\n",packet->length);
	//printf(" data= %s\n",packet->data);
#endif

	xSemaphoreGive(xIsTransmitting);

	return err;

}

int ChangeTrxvuConfigValues()
{
	if (logError(isis_vu_e__set_bitrate(0, isis_vu_e__bitrate__9600bps) ,"isis_vu_e__set_bitrate") ) return -1;
		if (logError(isis_vu_e__set_tx_freq(0, TX_FREQUENCY),"isis_vu_e__tx_freq") ) return -1;
		if (logError(isis_vu_e__set_tx_pll_powerout(0, 0xCFEF),"isis_vu_e__set_tx_pll_powerout") ) return -1;
		if (logError(isis_vu_e__set_rx_freq(0, RX_FREQUENCY), "isis_vu_e__rx_freq") ) return -1;
		if (logError(isis_vu_e__set_transponder_in_freq(0, RX_FREQUENCY), "isis_vu_e__set_transponder_in_freq") ) return -1;

	return E_NO_SS_ERR;
}

