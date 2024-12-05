#include "TrxvuTestingDemo.h"

#include <string.h>
#include <stdlib.h>

#include <hal/Utility/util.h>
#include <hal/Timing/Time.h>

#ifdef ISISEPS
	#include <satellite-subsystems/isismepsv2_ivid7_piu.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

#include "SubSystemModules/PowerManagment/EPS.h"	// for EPS_Conditioning

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/CommandDictionary.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/SPL.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/HashSecuredCMD.h"

Boolean TestInitTrxvu()
{
	int err = InitTrxvu();
	if(0 != err){
		printf("Error in 'InitTrxvy' = %d\n",err);
		return TRUE;
	}
	printf("init successful\n");
	return TRUE;
}

Boolean TestInitTrxvuWithDifferentFrameLengths()
{

//	ISIStrxvuI2CAddress myTRXVUAddress;
//	ISIStrxvuFrameLengths myTRXVUBuffers;
//	ISIStrxvuBitrate myTRXVUBitrates;
//
//	int retValInt = 0;
//	unsigned int bytes = 0;
//	printf("please enter size of Tx packet in bytes(0 for default):\n");
//	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&bytes,0,1000) == 0);
//	if(bytes == 0){
//		myTRXVUBuffers.maxAX25frameLengthTX = 235;
//	}else
//		myTRXVUBuffers.maxAX25frameLengthTX = bytes;
//
//	printf("please enter size of Rx packet in bytes(0 for default):\n");
//	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&bytes,0,1000) == 0);
//	if(bytes == 0){
//		myTRXVUBuffers.maxAX25frameLengthRX = 200;
//	}else
//		myTRXVUBuffers.maxAX25frameLengthRX = bytes;
//
//
//	myTRXVUAddress.addressVu_rc = I2C_TRXVU_RC_ADDR;
//	myTRXVUAddress.addressVu_tc = I2C_TRXVU_TC_ADDR;
//
//	myTRXVUBitrates = trxvu_bitrate_9600;
//
//	retValInt = IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUBuffers,
//			&myTRXVUBitrates, 1);
//	if (retValInt != 0) {
//		printf("error in Trxvi Init = %d\n",retValInt);
//		return retValInt;
//	}
//	vTaskDelay(100);

	return TRUE;
}

Boolean TestTrxvuLogic()
{
	int times = 0;
	int sleep = 0;
	int err = 0;
	printf("\nPlease insert number of times to run(0 to 1000)\n");
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&times,0,1000) == 0);

	printf("\nPlease sleep tick time (100 to 100,000)\n");
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&sleep,100,100000) == 0);

	err = isis_vu_e__set_bitrate(0, isis_vu_e__bitrate__9600bps);

	while(times > 0)
	{
		times--;

		err = TRX_Logic();
		if(0 != err){
			printf("error in TRX_Logic = %d\n exiting\n",err);
			return TRUE;
		}
		vTaskDelay(sleep);
	}
	return TRUE;
}

Boolean TestCheckTransmitionAllowed()
{
	char msg[10]  = {0};

	if(CheckTransmitionAllowed())
		strcpy(msg,"TRUE");
	else
		strcpy(msg,"FALSE");
	printf("Transmition is allowed: %s\n",msg);
	return TRUE;
}

Boolean TestLoopSPL()
{
	sat_packet_t packet = {0};
	packet.ID = 0xFF;
	packet.cmd_type = 0xAA;
	packet.cmd_subtype = 0xBB;

	char data[] = {0x01};
	for (int i=2;i<100;i++){
		data[0] = i;
		memcpy(packet.data,data,sizeof(data));

		packet.length = sizeof(data);

		int err = TransmitSplPacket(&packet,NULL);
		if(err != 0 ){
			printf("error in 'TransmitSplPacket' = %d\n",err);
		}
	}

	return TRUE;
}

Boolean TestSendDumpAbortRequest()
{
	printf("Sending Dump Abort Request\n");
	SendDumpAbortRequest();
	return TRUE;
}

Boolean TestTransmitDummySplPacket()
{
	sat_packet_t packet = {0};
	packet.ID = 0x12345678;
	packet.cmd_type = 0x42;
	packet.cmd_subtype = 0x43;

	char data[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};
	memcpy(packet.data,data,sizeof(data));

	packet.length = sizeof(data);

	int err = TransmitSplPacket(&packet,NULL);
	if(err != 0 ){
		printf("error in 'TransmitSplPacket' = %d\n",err);
	}
	return TRUE;
}

Boolean TestTransmitSplPacket()
{
	sat_packet_t packet = {0};
	packet.ID = 0x12345678;
	packet.cmd_type = 0x42;
	packet.cmd_subtype = 0x43;

	char data[MAX_COMMAND_DATA_LENGTH] = {0};

	for(unsigned int i = 0; i < MAX_COMMAND_DATA_LENGTH; i++)
	{
		data[i] = i;
	}
	packet.length = MAX_COMMAND_DATA_LENGTH;
	memcpy(packet.data,data,MAX_COMMAND_DATA_LENGTH);
	int minutes = 0;
		while(UTIL_DbguGetIntegerMinMax((unsigned int*)&minutes,0,20) == 0);

		time_unix curr_time = 0;
		Time_getUnixEpoch(&curr_time);

		time_unix end_time = MINUTES_TO_SECONDS(minutes) + curr_time;
		isis_vu_e__get_tx_telemetry_last__from_t tlm;
		isis_vu_e__get_tx_telemetry_last(ISIS_TRXVU_I2C_BUS_INDEX,&tlm);
		while(end_time > curr_time)
		{
			isis_vu_e__get_tx_telemetry_last(ISIS_TRXVU_I2C_BUS_INDEX,&tlm);
			if(tlm.fields.temp_board >=60)
				break;
			printf("board temperature: %d\n",tlm.fields.temp_board);
			Time_getUnixEpoch(&curr_time);

			TransmitSplPacket(&packet,NULL);

			EPS_Conditioning();

			printf("seconds t'ill end: %lu\n",end_time - curr_time);
			vTaskDelay(1000);
		}

	return TRUE;
}

Boolean TestExitDump()
{
	//AbortDump();
	return TRUE;
}

Boolean TestDumpTelemetry()
{
	sat_packet_t cmd = {0};
	cmd.cmd_type = 0;
	cmd.cmd_subtype = 0x69;
	cmd.ID = 0x02000008;
	cmd.length = 0x0D;
	char data[13] = {4, // dump type WOD
			0x80,0x85,0x74,0x67, // start time 1/1/2025
			0x80,0xCB,0x79,0x67, // end time 5/1/2025
			0,0,0,0}; // resultion 0
	memcpy(&cmd.data,data,sizeof(data));
	CMD_StartDump(&cmd);
	return TRUE;
}

Boolean TestRestoreDefaultBeaconParameters()
{
	/*
	printf("Restoring to default beacon parameters\n");

	time_unix beacon_interval_time = 0;
	FRAM_read((unsigned char*) &beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	printf("Value of interval before: %lu\n",beacon_interval_time);

	UpdateBeaconInterval(DEFAULT_BEACON_INTERVAL_TIME);

	FRAM_read((unsigned char*)&beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	printf("Value of interval after: %lu\n",beacon_interval_time);

	unsigned char cycle = 0;
	FRAM_read((unsigned char*) &cycle, BEACON_BITRATE_CYCLE_ADDR,
	BEACON_BITRATE_CYCLE_SIZE);
	printf("Value of beacon cycle before: %d\n",cycle);

	UpdateBeaconBaudCycle(DEFALUT_BEACON_BITRATE_CYCLE);

	FRAM_read((unsigned char*) &cycle, BEACON_BITRATE_CYCLE_ADDR,
	BEACON_BITRATE_CYCLE_SIZE);
	printf("Value of beacon cycle after: %d\n",cycle);
*/
	return TRUE;
}

Boolean TestChooseDefaultBeaconCycle()
{
	printf("Please state new beacon interval in seconds(0 to cancel; min 5, max 60):\n");

	unsigned int seconds = 0;
	while(UTIL_DbguGetIntegerMinMax(&seconds,5,60) == 0);
	if(0 == seconds){
		return TRUE;
	}
	time_unix beacon_interval_time = 0;
	FRAM_read((unsigned char*) &beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	printf("Value before: %lu\n",beacon_interval_time);

	beacon_interval_time = seconds;
	FRAM_write((unsigned char*) &beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);

	FRAM_read((unsigned char*) &beacon_interval_time, BEACON_INTERVAL_TIME_ADDR,
	BEACON_INTERVAL_TIME_SIZE);
	printf("Value after: %lu\n",beacon_interval_time);

	return TRUE;
}

Boolean TestBeaconLogic()
{
	printf("Please state number of minutes to perform the test(max 20, 0 to cancel):\n");

	int minutes = 0;
	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&minutes,0,20) == 0);

	time_unix curr_time = 0;
	Time_getUnixEpoch(&curr_time);

	time_unix end_time = MINUTES_TO_SECONDS(minutes) + curr_time;

	while(end_time > curr_time)
	{
		Time_getUnixEpoch(&curr_time);

		BeaconLogic(FALSE);

		EPS_Conditioning();

		printf("seconds t'ill end: %lu\n",end_time - curr_time);
		vTaskDelay(1000);
	}
	return TRUE;
}

Boolean TestMuteTrxvu()
{
	time_unix duration = 3;
	printf("Please state number of minutes to perform the test(max 10):");

	while(UTIL_DbguGetIntegerMinMax((unsigned int*)&duration,0,10) == 0);

	int err = muteTRXVU(duration);
	if(0 != err){
		printf("error in 'muteTRXVU' = %d\n",err);
		return TRUE;
	}

	time_unix curr = 0;
	Time_getUnixEpoch(&curr);

#ifdef ISISEPS
	//ieps_statcmd_t cmd;
#endif
	/*
	while(!CheckForMuteEnd()){

		printf("current tick = %d\n",(int)xTaskGetTickCount());

#ifdef ISISEPS
		IsisEPS_resetWDT(EPS_I2C_BUS_INDEX,&cmd);
#endif
#ifdef GOMEPS
		GomEpsResetWDT(EPS_I2C_BUS_INDEX);
#endif
		printf("sending ACK(if transmission was heard then error :/ )\n");
		SendAckPacket(ACK_MUTE,NULL,NULL,0);
		vTaskDelay(1000);
	}
	UnMuteTRXVU();
	Boolean mute_flag = GetMuteFlag();
	SendAckPacket(ACK_MUTE,NULL,(unsigned char*)&mute_flag,sizeof(mute_flag));
	*/
	return TRUE;
}

Boolean TestUnMuteTrxvu()
{
	printf("Stopping mute. tick = %d\n",(int)xTaskGetTickCount());
	UnMuteTRXVU();
	return TRUE;
}

Boolean TestGetNumberOfFramesInBuffer()
{
	int num_of_frames = GetNumberOfFramesInBuffer();
	if(-1 == num_of_frames){
		printf("error in 'GetNumberOfFramesInBuffer'\n");
		return TRUE;
	}
	printf("Number of frames in the online buffer = %d\n", num_of_frames);
	return TRUE;
}

Boolean TestSetTrxvuBitrate()
{
	int err = 0;
	isis_vu_e__bitrate_t bitrate = 0;
	unsigned int index = 0;
	printf("Choose bitrate:\n \t(0)Cancel\n\t(1) = 1200\n\t(2) = 2400\n\t(3) = 4800\n\t(4) = 9600\n");

	while (UTIL_DbguGetIntegerMinMax(&index, 0, 4) == 0);

	switch(index)
	{
	case 0:
		break;
	case 1:
		bitrate = isis_vu_e__bitrate__1200bps;
		break;
	case 2:
		bitrate = isis_vu_e__bitrate__2400bps;
			break;
	case 3:
		bitrate = isis_vu_e__bitrate__4800bps;
			break;
	case 4:
		bitrate = isis_vu_e__bitrate__9600bps;
			break;
	}

	err = isis_vu_e__set_bitrate(ISIS_TRXVU_I2C_BUS_INDEX,bitrate);
	if(0 != err){
		printf("error in 'IsisTrxvu_tcSetAx25Bitrate' = %d",err);
		return TRUE;
	}
	return TRUE;
}



#define MAX_INPUT_SIZE 1024  // Maximum size of input string
Boolean testSecuredCMD() {
    char input[MAX_INPUT_SIZE];
    BYTE buf[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;

    // Get user input
    printf("Enter a string to hash: \n");

    // Use scanf to get the input (with a width specifier to avoid overflow)
    scanf("%1023s", input);  // Reads up to MAX_INPUT_SIZE - 1 to prevent overflow

    // Remove newline character if present (scanf stops at whitespace, so this part may not be necessary)
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    // Initialize SHA256 context
    sha256_init(&ctx);

    // Hash the user input
    sha256_update(&ctx, (BYTE*)input, strlen(input));
    sha256_final(&ctx, buf);

    // Print the resulting hash
    printf("SHA-256 hash: ");
    for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");

    return TRUE;
}

int Hash256(char* text, BYTE* outputHash)
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

    return E_NO_SS_ERR;
}

Boolean Dummy_CMD_Hash256(sat_packet_t *cmd)
{
unsigned int code, lastid, currId;
char plsHashMe[50];
char code_to_str[50];
char cmpHash[Max_Hash_size], temp[Max_Hash_size];
currId = cmd->ID;

// Debug print: Initial command ID
printf("DEBUG: Initial cmd->ID = %u\n", currId);

// Get code from FRAM
FRAM_read((unsigned char*)&code, CMD_Passcode_ADDR, CMD_Passcode_SIZE);
// Debug print: Passcode from FRAM
printf("DEBUG: Passcode read from FRAM = %u\n", code);

// Get the last ID from FRAM and save it into variable `lastid`
FRAM_read((unsigned char*)&lastid, CMD_ID_ADDR, CMD_ID_SIZE);
// Debug print: Last ID from FRAM
printf("DEBUG: Last ID read from FRAM = %u\n", lastid);

// Write new ID to FRAM as the last ID
FRAM_write((unsigned char*)&currId, CMD_ID_ADDR, CMD_ID_SIZE);

// Check if the current ID is bigger than the last ID
if (currId <= lastid) {
    printf("DEBUG: Current ID is not greater than last ID. Authorization failed.\n");
    return -1;//TODO E_UNAUTHORIZED;
}

// Combine `lastid` (as a string) into `plsHashMe`
sprintf(plsHashMe, "%u", currId);
// Debug print: Combined string in `plsHashMe`
printf("DEBUG: Combined string (plsHashMe) = %s\n", plsHashMe);

// Turn `code` into a string
sprintf(code_to_str, "%u", code);
// Debug print: Passcode as string
printf("DEBUG: Passcode as string (code_to_str) = %s\n", code_to_str);

// Append passcode to `plsHashMe`
strcat(plsHashMe, code_to_str);
// Debug print: Final string to hash
printf("DEBUG: Final string to hash (plsHashMe) = %s\n", plsHashMe);

// Initialize buffer for hashed output
BYTE hashed[SHA256_BLOCK_SIZE];

// Hash the combined string
int err = Hash256(plsHashMe, hashed);
if (err != E_NO_SS_ERR) {
    printf("DEBUG: Hashing failed with error code %d\n", err);
    return FALSE;
}

// Debug print: Hashed output
printf("DEBUG: Hashed output = ");
for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
    printf("%02x", hashed[i]);
}
printf("\n");

// Copy byte-by-byte to `temp` and convert to hex string
char otherhashed[Max_Hash_size * 2 + 1]; // Array to store 8 bytes in hex, plus a null terminator
for (int i = 0; i < Max_Hash_size; i++) {
    sprintf(&otherhashed[i * 2], "%02x", hashed[i]);
}
otherhashed[16] = '\0'; // Add null terminator

// Debug print: Hexadecimal representation of the hash
printf("DEBUG: Hexadecimal hash (otherhashed) = %s\n", otherhashed);

// Copy the first 8 bytes to `temp`
memcpy(temp, otherhashed, Max_Hash_size);
// Debug print: Temp hash for comparison
printf("DEBUG: Temp hash for comparison = %s\n", temp);

// Copy the first 8 bytes of the data
memcpy(cmpHash, cmd->data, Max_Hash_size);
// Debug print: Command hash to compare
printf("DEBUG: Command hash to compare (cmpHash) = %s\n", cmpHash);

if (cmd->length < Max_Hash_size) {
    printf("DEBUG: Command data length is less than the hash size. Memory allocation error.\n");
    return E_MEM_ALLOC;
}
cmd->data[9]=8;
cmd->data[10]=8;
cmd->data[11]=8;
cmd->data[12]=8;
cmd->data[13]=8;
cmd->data[14]=8;
cmd->data[15]=8;
cmd->data[16]=8;
// Adjust command data
cmd->length -= Max_Hash_size; // 8 bytes are removed from the data; reflect this in the length
memmove(cmd->data, cmd->data + Max_Hash_size, cmd->length);
// Debug print: Adjusted command data length
printf("DEBUG: Adjusted command data length = %u\n", cmd->length);

// Compare hashes
if (memcmp(temp, cmpHash, Max_Hash_size) == 0) {
    printf("DEBUG: Hashes match! Success!\n");
    return TRUE;
} else {
    printf("DEBUG: Hashes do not match. Authorization failed.\n");
    return -1;//TODO E_UNAUTHORIZED;
}

}

Boolean CMD_Hash256(sat_packet_t *cmd)
{
unsigned int code, lastid, currId;
    char plsHashMe[50];
    char code_to_str[50];
    char cmpHash[Max_Hash_size], temp[Max_Hash_size];
    currId = cmd->ID;

	if (cmd == NULL || cmd->data == NULL) {
		return E_INPUT_POINTER_NULL;
	}

    //get code from FRAM
    FRAM_read((unsigned char*)&code, CMD_Passcode_ADDR, CMD_Passcode_SIZE);

    //get the last id from FRAM and save it into var lastid then add new id to the FRAM (as new lastid)
    FRAM_read((unsigned char*)&lastid, CMD_ID_ADDR, CMD_ID_SIZE);
    FRAM_write((unsigned char*)&currId, CMD_ID_ADDR, CMD_ID_SIZE);

    //check if curr ID is bigger than lastid
    if(currId <= lastid)
    {
        return -1;//TODO E_UNAUTHORIZED;//bc bool FALSE needed?
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
    int err = Hash256(plsHashMe, hashed);
    if (err != E_NO_SS_ERR) {
	//add to log?
        return FALSE;
    }
     
    //cpy byte by byte to temp (size of otherhashed = 8 bytes *2 (all bytes are saved by twos(bc its in hex))+1 for null)
    char otherhashed[Max_Hash_size * 2 + 1]; // Array to store 8 bytes in hex, plus a null terminator

    for (int i = 0; i < Max_Hash_size; i++) {
        sprintf(&otherhashed[i * 2], "%02x", hashed[i]);
    }
    otherhashed[16] = '\0'; // Add Null

    //cpy first 8 bytes to temp 
    memcpy(temp, otherhashed, Max_Hash_size);

    //cpy first 8 bytes of the data
    memcpy(cmpHash, cmd -> data, Max_Hash_size);

	if(cmd -> length < Max_Hash_size)
		return E_MEM_ALLOC;

	//fix cmd.data
	cmd -> length = cmd -> length - Max_Hash_size;//8 bytes are removed from the data this must be reflected in the length
	memmove(cmd->data, cmd->data + Max_Hash_size,cmd->length - Max_Hash_size);
	
    //cmp hash from command centre to internal hash
    if(memcmp(temp, cmpHash, Max_Hash_size) == 0)
    {   
        printf("success!\n");//for test
        return TRUE;
    }
    else
        return -1;//TODO E_UNAUTHORIZED;

}

Boolean TestForDummy_sat_packet()
{
	sat_packet_t cmd;
	int err;
	unsigned int passcode = 1;
	char hash[Max_Hash_size + 9];
	cmd.ID = 2;
	cmd.cmd_type = trxvu_cmd_type;
	cmd.cmd_subtype = SecuredCMD;
	cmd.length = Max_Hash_size * 2;
	unsigned int one = 1;
	sprintf(hash, "%s", "6f4b661212345678");
	memcpy(&cmd.data, &hash, Max_Hash_size);
	FRAM_write((unsigned char*)&one, CMD_ID_ADDR, CMD_ID_SIZE);
	FRAM_write((unsigned char*)&passcode, CMD_Passcode_ADDR, CMD_Passcode_SIZE);
	err = Dummy_CMD_Hash256(&cmd);
	return err;
}

Boolean Secured_CMD_TEST()
{
	sat_packet_t cmd;
	//unsigned int passcode;
	char hash[Max_Hash_size + 9]; //9 added for breathing space
	cmd.ID = 2;
	cmd.cmd_type = trxvu_cmd_type;
	cmd.cmd_subtype = SecuredCMD;
	cmd.length = Max_Hash_size * 2;//* 2 added bc I keep switching the hash (makes it easyer)
	unsigned int one = 1;
	sprintf(hash, "%s", "6f4b661212345678");
	memcpy(&cmd.data, &hash, Max_Hash_size);
	FRAM_write((unsigned char*)&one, CMD_ID_ADDR, CMD_ID_SIZE);
	FRAM_write((unsigned char*)&one, CMD_Passcode_ADDR, CMD_Passcode_SIZE);
	int err;
	err = ActUponCommand(&cmd);

	return err;
}

Boolean  dumpRamTest()
{
	sat_packet_t cmd;
	cmd.ID = 8;
	cmd.cmd_subtype = DUMP_RAM_TLM;
	cmd.cmd_type = trxvu_cmd_type;
	unsigned char data[10] = "hii";
	memcpy(cmd.data, data, sizeof(data));
	cmd.length = sizeof(cmd.data);
	ActUponCommand(&cmd);

	return TRUE;
}

Boolean TestGetTrxvuBitrate()
{
//	int err = 0;
//	ISIStrxvuBitrateStatus bitrate = 0;
//	//err = GetTrxvuBitrate(&bitrate);
//	if(0 != err){
//		printf("error in 'GetTrxvuBitrate' = %d\n",err);
//		return TRUE;
//	}
//	switch(bitrate)
//	{
//	case trxvu_bitratestatus_1200:
//		printf("bitrate is 'trxvu_bitratestatus_1200'\n");
//		break;
//	case trxvu_bitratestatus_2400:
//		printf("bitrate is 'trxvu_bitratestatus_2400'\n");
//			break;
//	case trxvu_bitratestatus_4800:
//		printf("bitrate is 'trxvu_bitratestatus_4800'\n");
//			break;
//	case trxvu_bitratestatus_9600:
//		printf("bitrate is 'trxvu_bitratestatus_9600'\n");
//			break;
//	}
	return TRUE;
}

Boolean TestTransmitDataAsSPL_Packet()
{
	sat_packet_t cmd = {0};
	cmd.ID = 0x42;
	cmd.cmd_type = 0x43;
	cmd.cmd_subtype = 0x44;

	unsigned char data[] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10};
	printf("Transmitted data\n");
	for(unsigned int i =0; i < sizeof(data);i++){
		printf("%X\t",data[i]);
	}
	printf("\n\n");
	int err = TransmitDataAsSPL_Packet(&cmd,data,sizeof(data));
	if(0 != err){
		printf("error in 'TransmitDataAsSPL_Packet' = %d\n",err);
		return TRUE;
	}
	return TRUE;
}

Boolean selectAndExecuteTrxvuDemoTest()
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Init Trxvu \n\r");
	printf("\t 2) Init Trxvu With different Fram Lengths\n\r");
	printf("\t 3) Test Trxvu Logic \n\r");
	printf("\t 4) Send Dump Abort Request using Queue\n\r");
	printf("\t 5) Transmit Dummy SPL Packet\n\r");
	printf("\t 6) Transmit SPL Packet\n\r");
	printf("\t 7) Exit Dump\n\r");
	printf("\t 8) Test Dump\n\r");
	printf("\t 9) Test Beacon Logic\n\r");
	printf("\t 10) Mute Trxvu\n\r");
	printf("\t 11) Unmute Trxvu\n\r");
	printf("\t 12) Get Number Of Frames In Buffer\n\r");
	printf("\t 13) Set Trxvu Bitrate\n\r");
	printf("\t 14) Get Trxvu Bitrate\n\r");
	printf("\t 15) Transmit Data As SPL Packet\n\r");
	printf("\t 16) Change beacon inervals\n\r");
	printf("\t 17) Restore to default beacon inervals\n\r");
	printf("\t 18) Check Transmition Allowed\n\r");
	printf("\t 19) Loop Transmition of SPL\n\r");
	printf("\t 20) Dump RAM\n\r");
	printf("\t 21) haash test for Secured CMD\n\r");
	printf("\t 22) dummy Secured CMD\n\r");
	printf("\t 23) Secured CMD test \n\r");

	unsigned int number_of_tests = 23;
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = TestInitTrxvu();
		break;
	case 2:
		offerMoreTests = TestInitTrxvuWithDifferentFrameLengths();
		break;
	case 3:
		offerMoreTests = TestTrxvuLogic();
		break;
	case 4:
		offerMoreTests = TestSendDumpAbortRequest();
		break;
	case 5:
		offerMoreTests = TestTransmitDummySplPacket();
		break;
	case 6:
		offerMoreTests = TestTransmitSplPacket();
		break;
	case 7:
		offerMoreTests = TestExitDump();
		break;
	case 8:
		offerMoreTests = TestDumpTelemetry();
		break;
	case 9:
		offerMoreTests = TestBeaconLogic();
		break;
	case 10:
		offerMoreTests = TestMuteTrxvu();
		break;
	case 11:
		offerMoreTests = TestUnMuteTrxvu();
		break;
	case 12:
		offerMoreTests = TestGetNumberOfFramesInBuffer();
		break;
	case 13:
		offerMoreTests = TestSetTrxvuBitrate();
		break;
	case 14:
		offerMoreTests = TestGetTrxvuBitrate();
		break;
	case 15:
		offerMoreTests = TestTransmitDataAsSPL_Packet();
		break;
	case 16:
		offerMoreTests = TestChooseDefaultBeaconCycle();
		break;
	case 17:
		offerMoreTests = TestRestoreDefaultBeaconParameters();
		break;
	case 18:
		offerMoreTests = TestCheckTransmitionAllowed();
		break;
	case 19:
		offerMoreTests = TestLoopSPL();
		break;
	case 20:
		offerMoreTests = dumpRamTest();
		break;
	case 21:
		offerMoreTests = testSecuredCMD();
		break;
	case 22:
		offerMoreTests = TestForDummy_sat_packet();
		break;
	case 23:
		offerMoreTests = Secured_CMD_TEST();
		break;
	default:
		break;
	}
	return offerMoreTests;
}

Boolean MainTrxvuTestBench()
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTrxvuDemoTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return FALSE;
}
