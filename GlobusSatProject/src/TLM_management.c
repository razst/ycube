/*
 * filesystem.c
 *
 *  Created on: 2020
 *      Author: Yerucham satellite team.
 */


//#include <at91/utility/trace.h>
//#include <GlobalStandards.h>
//#include <hal/errors.h>
//#include <hal/Storage/FRAM.h>
#include <hal/Timing/Time.h>
#include <hcc/api_fat.h>
#include <hcc/api_hcc_mem.h>
#include <SubSystemModules/Housekepping/TelemetryFiles.h>
#include <SubSystemModules/Housekepping/TelemetryCollector.h>
#include <SubSystemModules/Communication/SatCommandHandler.h>
#include <SubSystemModules/Communication/SPL.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <stdio.h>
//#include <SubSystemModules/Communication/SPL.h>
#include <TLM_management.h>
#include <utils.h>
#include <string.h>
#include <time.h>


#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/isis_eps_driver.h>

#define SKIP_FILE_TIME_SEC 1000000
#define SD_CARD_DRIVER_PARMS 0
#define FIRST_TIME -1

#define NUM_ELEMENTS_READ_AT_ONCE 400 // TODO check if 400 is the right number !!!




void delete_allTMFilesFromSD()
{
	F_FIND find;
	if (!f_findfirst("A:/*.*",&find))
	{
		do
		{
			f_delete(find.filename);
		} while (!f_findnext(&find));
	}
}

FileSystemResult InitializeFS(Boolean first_time)
{

	// Initialize the memory for the FS
	if(logError(hcc_mem_init())) return -1;

	// Initialize the FS
	if(logError(fs_init())) return -1;

	// Tell the OS (freeRTOS) about our FS
	if(logError(f_enterFS())) return -1;

	// Initialize the volume of SD card 0 (A)
	// TODO should we also init the volume of SD card 1 (B)???
	if(logError(f_initvolume( 0, atmel_mcipdc_initfunc, SD_CARD_DRIVER_PARMS ))) return -1;

	//In the first time the SD on. if there is file on the SD delete it.
	if(first_time) delete_allTMFilesFromSD();

	return FS_SUCCSESS;
}


//TODO when we get the Sat, check if we get 01 or 1 for the day and update the code
void calculateFileName(Time curr_date,char* file_name, char* endFileName, int dayBack)
{
	/* initialize */
	struct tm t = { .tm_year = curr_date.year + 100, .tm_mon = curr_date.month - 1, .tm_mday = curr_date.date };
	/* modify */
	t.tm_mday += dayBack;
	mktime(&t);

	char buff[7];
	strftime(buff, sizeof buff, "%y%0m%0d", &t);
	snprintf(file_name, MAX_FILE_NAME_SIZE, "%s.%s", buff, endFileName);
}

int write2File(void* data, tlm_type_t tlmType){
	// TODO what happens if there was an error writing to SD, the whole file will be corrupted for us.
	printf("writing tlm: %d to SD\n",tlmType);

	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);

	Time curr_date;
	Time_get(&curr_date);

	int size;
	F_FILE *fp;
	char file_name[MAX_FILE_NAME_SIZE] = {0};


	if (tlmType==tlm_tx_revc){
		calculateFileName(curr_date,&file_name, &END_FILE_NAME_TX_REVC, 0);
		fp = f_open(file_name, "a");
		size = sizeof(ISIStrxvuTxTelemetry_revC);
	}
	else if (tlmType==tlm_tx){
		calculateFileName(curr_date,&file_name, &END_FILE_NAME_TX, 0);
		fp = f_open(file_name, "a");
		size = sizeof(ISIStrxvuTxTelemetry);
	}
	else if (tlmType==tlm_rx){
		calculateFileName(curr_date,&file_name, &END_FILE_NAME_RX, 0);
		fp = f_open(file_name, "a");
		size = sizeof(ISIStrxvuRxTelemetry);
	}
	else if (tlmType==tlm_rx_revc){
		calculateFileName(curr_date,&file_name, &END_FILE_NAME_RX_REVC, 0);
		fp = f_open(file_name, "a");
		size = sizeof(ISIStrxvuRxTelemetry_revC);
	}
	else if (tlmType==tlm_rx_frame){
		calculateFileName(curr_date,&file_name, &END_FILE_NAME_RX_FRAME, 0);
		fp = f_open(file_name, "a");
		size = sizeof(ISIStrxvuRxFrame);
	}
	else if (tlmType==tlm_antenna){
		calculateFileName(curr_date,&file_name, &END_FILE_NAME_ANTENNA, 0);
		fp = f_open(file_name, "a");
		size = sizeof(ISISantsTelemetry);
	}
	else if (tlmType==tlm_eps_raw_mb){
		calculateFileName(curr_date,&file_name, &END_FILENAME_EPS_RAW_MB_TLM, 0);
		fp = f_open(file_name, "a");
		size = sizeof(isis_eps__gethousekeepingraw__from_t);
	}
	else if (tlmType==tlm_eps_raw_cdb){
		calculateFileName(curr_date,&file_name, &END_FILENAME_EPS_RAW_CDB_TLM, 0);
		fp = f_open(file_name, "a");
		size = sizeof(isis_eps__gethousekeepingrawincdb__from_t);
	}
	else if (tlmType==tlm_eps_eng_mb){
		calculateFileName(curr_date,&file_name, &END_FILENAME_EPS_ENG_MB_TLM, 0);
		fp = f_open(file_name, "a");
		size = sizeof(isis_eps__gethousekeepingeng__from_t);
	}
	else if (tlmType==tlm_eps_eng_cdb){
		calculateFileName(curr_date,&file_name, &END_FILENAME_EPS_ENG_CDB_TLM, 0);
		fp = f_open(file_name, "a");
		size = sizeof(isis_eps__gethousekeepingengincdb__from_t);
	}
	else if (tlmType==tlm_wod){
		calculateFileName(curr_date,&file_name, &END_FILENAME_WOD_TLM, 0);
		fp = f_open(file_name, "a");
		size = sizeof(WOD_Telemetry_t);
	}
	else if (tlmType==tlm_solar){
		calculateFileName(curr_date,&file_name, &END_FILENAME_SOLAR_PANELS_TLM, 0);
		fp = f_open(file_name, "a");
		size = sizeof(solar_tlm_t);
	}
	else if (tlmType==tlm_log){
		calculateFileName(curr_date,&file_name, &END_FILENAME_LOGS, 0);
		fp = f_open(file_name, "a");
		size = sizeof(logData_t);
	}




	if (!fp)
	{
		printf("Unable to open file!");
		return 1;
	}

	f_write(&curr_time , sizeof(curr_time) ,1, fp );
	f_write(data , size , 1, fp );

	/* close the file*/
	f_flush(fp);
	f_close (fp);
	return 0;
}


int readTLMFile(tlm_type_t tlmType, Time date, int numOfDays,sat_packet_t *cmd){
	//TODO check for unsupported tlmType
	printf("reading from file...\n");

	unsigned int offset = 0;

	FILE * fp;
	int size=0;
	char file_name[MAX_FILE_NAME_SIZE] = {0};


	// TODO: put all the if's... in a function - use also in write2file
	if (tlmType==tlm_tx_revc){
		calculateFileName(date,&file_name, &END_FILE_NAME_TX_REVC, 0);
		fp = f_open(file_name, "r");
		size = sizeof(ISIStrxvuTxTelemetry_revC);
	}
	else if (tlmType==tlm_tx){
		calculateFileName(date,&file_name, &END_FILE_NAME_TX, 0);
		fp = f_open(file_name, "r");
		size = sizeof(ISIStrxvuTxTelemetry);
	}
	else if (tlmType==tlm_rx){
		calculateFileName(date,&file_name, &END_FILE_NAME_RX, 0);
		fp = f_open(file_name, "r");
		size = sizeof(ISIStrxvuRxTelemetry);
	}
	else if (tlmType==tlm_rx_revc){
		calculateFileName(date,&file_name, &END_FILE_NAME_RX_REVC, 0);
		fp = f_open(file_name, "r");
		size = sizeof(ISIStrxvuRxTelemetry_revC);
	}
	else if (tlmType==tlm_rx_frame){
		calculateFileName(date,&file_name, &END_FILE_NAME_RX_FRAME, 0);
		fp = f_open(file_name, "r");
		size = sizeof(ISIStrxvuRxFrame);
	}
	else if (tlmType==tlm_antenna){
		calculateFileName(date,&file_name, &END_FILE_NAME_ANTENNA, 0);
		fp = f_open(file_name, "r");
		size = sizeof(ISISantsTelemetry);
	}
	else if (tlmType==tlm_eps_raw_mb){
		calculateFileName(date,&file_name, &END_FILENAME_EPS_RAW_MB_TLM, 0);
		fp = f_open(file_name, "r");
		size = sizeof(isis_eps__gethousekeepingraw__from_t);
	}
	else if (tlmType==tlm_eps_raw_cdb){
		calculateFileName(date,&file_name, &END_FILENAME_EPS_RAW_CDB_TLM, 0);
		fp = f_open(file_name, "r");
		size = sizeof(isis_eps__gethousekeepingrawincdb__from_t);
	}
	else if (tlmType==tlm_eps_eng_mb){
		calculateFileName(date,&file_name, &END_FILENAME_EPS_ENG_MB_TLM, 0);
		fp = f_open(file_name, "r");
		size = sizeof(isis_eps__gethousekeepingeng__from_t);
	}
	else if (tlmType==tlm_eps_eng_cdb){
		calculateFileName(date,&file_name, &END_FILENAME_EPS_ENG_CDB_TLM, 0);
		fp = f_open(file_name, "r");
		size = sizeof(isis_eps__gethousekeepingengincdb__from_t);
	}
	else if (tlmType==tlm_wod){
		calculateFileName(date,&file_name, &END_FILENAME_WOD_TLM, 0);
		fp = f_open(file_name, "r");
		size = sizeof(WOD_Telemetry_t);
	}
	else if (tlmType==tlm_solar){
		calculateFileName(date,&file_name, &END_FILENAME_SOLAR_PANELS_TLM, 0);
		fp = f_open(file_name, "a");
		size = sizeof(solar_tlm_t);
	}
	else if (tlmType==tlm_log){
		calculateFileName(date,&file_name, &END_FILENAME_LOGS, 0);
		fp = f_open(file_name, "a");
		size = sizeof(logData_t);
	}


	if (!fp)
	{
		printf("Unable to open file!");// TODO: log error in all printf in the file!
		return 1;
	}


	char element[(sizeof(int)+size)];// buffer for a single element that we will tx
	char buffer[(sizeof(element)) * NUM_ELEMENTS_READ_AT_ONCE]; // buffer for data coming from SD (time+size of data struct)
	int numOfElementsSent=0;

	while(1)
	{
		int readElemnts = f_read(&buffer , sizeof(int)+size , NUM_ELEMENTS_READ_AT_ONCE, fp );
		offset = 0;
		if(!readElemnts) break;

		for (;readElemnts>0;readElemnts--){

			memcpy( &element, buffer + offset, sizeof(int) );
			printf("Tlm time is:%d\n",element);
			offset += sizeof(int);

			memcpy ( &element+offset, buffer + offset, size );
			//printf("Data = %d,%f\n",epsData.satState,epsData.vBat);
			offset += size;

			sat_packet_t dump_tlm = { 0 };


			AssembleCommand((unsigned char*)element, sizeof(int)+size,
					(char) DUMP_SUBTYPE, (char) (tlmType),
					cmd->ID, &dump_tlm);


			TransmitSplPacket(&dump_tlm, NULL);
			numOfElementsSent++;

		}// end for loop...

	}// while (1) loop...

	/* close the file*/
	f_close (fp);
	return numOfElementsSent;
}



	int readTLMFiles(tlm_type_t tlmType, Time date, int numOfDays,sat_packet_t *cmd){
		for(int i = 0; i < numOfDays; i++){
			readTLMFile(tlmType, date, i,cmd);
		}

		return 0;
	}


	void DeInitializeFS( void )
	{
	}
