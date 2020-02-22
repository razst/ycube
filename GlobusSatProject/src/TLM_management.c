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
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <stdio.h>
//#include <SubSystemModules/Communication/SPL.h>
#include <TLM_management.h>
#include <utils.h>
#include <string.h>
#include <time.h>


#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/IsisEPS.h>

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
	printf("writing 2 file...\n");

	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);

	Time curr_date;
	Time_get(&curr_date);

	int size;
	F_FILE *fp;
	char file_name[MAX_FILE_NAME_SIZE] = {0};

	/* open the file for writing in append mode*/
	//	if (tlmType==EPS){
	//		fp = f_open(calculateFileName(curr_date, "eps", 0), "a");
	//		size = sizeof(EPS_TLM);
	//	}else if (tlmType==TRXVU){
	//		fp = f_open(calculateFileName(curr_date, "trx", 0), "a");
	//		size = sizeof(TRXVU_TLM);
	//	}
/*
	if (tlmType==tlm_tx_revc){
		fp = f_open(calculateFileName(curr_date, END_FILE_NAME_TX_REVC, 0), "a");
		size = sizeof(ISIStrxvuTxTelemetry_revC);
	}
	else if (tlmType==tlm_tx){
		fp = f_open(calculateFileName(curr_date, END_FILE_NAME_TX, 0), "a");
		size = sizeof(ISIStrxvuTxTelemetry);
	}
	else if (tlmType==tlm_rx){
		fp = f_open(calculateFileName(curr_date, END_FILE_NAME_RX, 0), "a");
		size = sizeof(ISIStrxvuRxTelemetry);
	}
	else if (tlmType==tlm_rx_revc){
		fp = f_open(calculateFileName(curr_date, END_FILE_NAME_RX_REVC, 0), "a");
		size = sizeof(ISIStrxvuRxTelemetry_revC);
	}
	else if (tlmType==tlm_rx_frame){
		fp = f_open(calculateFileName(curr_date, END_FILE_NAME_RX_FRAME, 0), "a");
		size = sizeof(ISIStrxvuRxFrame);
	}
	else if (tlmType==tlm_antenna){
		fp = f_open(calculateFileName(curr_date, END_FILE_NAME_ANTENNA, 0), "a");
		size = sizeof(ISISantsTelemetry);
	}
	else if (tlmType==tlm_eps_raw_mb){
		fp = f_open(calculateFileName(curr_date, END_FILENAME_EPS_RAW_MB_TLM, 0), "a");
		size = sizeof(ieps_rawhk_data_mb_t);
	}
	else if (tlmType==tlm_eps_raw_cdb){
		fp = f_open(calculateFileName(curr_date, END_FILENAME_EPS_RAW_CDB_TLM, 0), "a");
		size = sizeof(ieps_rawhk_data_cdb_t);
	}
	else if (tlmType==tlm_eps_eng_mb){
		fp = f_open(calculateFileName(curr_date, END_FILENAME_EPS_ENG_MB_TLM, 0), "a");
		size = sizeof(ieps_enghk_data_mb_t);
	}
	else if (tlmType==tlm_eps_eng_cdb){
		fp = f_open(calculateFileName(curr_date, END_FILENAME_EPS_ENG_CDB_TLM, 0), "a");
		size = sizeof(ieps_enghk_data_cdb_t);
	}
	else if (tlmType==tlm_wod){
		fp = f_open(calculateFileName(curr_date, END_FILENAME_WOD_TLM, 0), "a");
		size = sizeof(WOD_Telemetry_t);
	}
	else */if (tlmType==tlm_solar){
		calculateFileName(curr_date,&file_name, &END_FILENAME_SOLAR_PANELS_TLM, 0);
		fp = f_open(file_name, "a");
		size = sizeof(solar_tlm_t);
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


int readTLMFile(tlm_type_t tlmType, Time date, int numOfDays){
	//TODO check for unsupported tlmType
	printf("reading from file...\n");

	unsigned int current_time;
	Time_getUnixEpoch(&current_time);


	ISIStrxvuTxTelemetry_revC txRevcData;
	ISIStrxvuTxTelemetry txData;
	ISIStrxvuRxTelemetry rxData;
	ISIStrxvuRxTelemetry_revC rxRevcData;
	ISIStrxvuRxFrame rxFrameData;
	ISISantsTelemetry antData;
	ieps_rawhk_data_mb_t rawMbData;
	ieps_rawhk_data_cdb_t rawCdbData;
	ieps_enghk_data_mb_t engMbData;
	ieps_enghk_data_cdb_t engCdbData;
	WOD_Telemetry_t wodData;
	solar_tlm_t solarData;


	unsigned int offset = 0;

	FILE * fp;
	int size=0;

/*
	if (tlmType==tlm_tx_revc){
		fp = f_open(calculateFileName(date, END_FILE_NAME_TX_REVC, 0), "r");
		size = sizeof(ISIStrxvuTxTelemetry_revC);
	}
	else if (tlmType==tlm_tx){
		fp = f_open(calculateFileName(date, END_FILE_NAME_TX, 0), "r");
		size = sizeof(ISIStrxvuTxTelemetry);
	}
	else if (tlmType==tlm_rx){
		fp = f_open(calculateFileName(date, END_FILE_NAME_RX, 0), "r");
		size = sizeof(ISIStrxvuRxTelemetry);
	}
	else if (tlmType==tlm_rx_revc){
		fp = f_open(calculateFileName(date, END_FILE_NAME_RX_REVC, 0), "r");
		size = sizeof(ISIStrxvuRxTelemetry_revC);
	}
	else if (tlmType==tlm_rx_frame){
		fp = f_open(calculateFileName(date, END_FILE_NAME_RX_FRAME, 0), "r");
		size = sizeof(ISIStrxvuRxFrame);
	}
	else if (tlmType==tlm_antenna){
		fp = f_open(calculateFileName(date, END_FILE_NAME_ANTENNA, 0), "r");
		size = sizeof(ISISantsTelemetry);
	}
	else if (tlmType==tlm_eps_raw_mb){
		fp = f_open(calculateFileName(date, END_FILENAME_EPS_RAW_MB_TLM, 0), "r");
		size = sizeof(ieps_rawhk_data_mb_t);
	}
	else if (tlmType==tlm_eps_raw_cdb){
		fp = f_open(calculateFileName(date, END_FILENAME_EPS_RAW_CDB_TLM, 0), "r");
		size = sizeof(ieps_rawhk_data_cdb_t);
	}
	else if (tlmType==tlm_eps_eng_mb){
		fp = f_open(calculateFileName(date, END_FILENAME_EPS_ENG_MB_TLM, 0), "r");
		size = sizeof(ieps_enghk_data_mb_t);
	}
	else if (tlmType==tlm_eps_eng_cdb){
		fp = f_open(calculateFileName(date, END_FILENAME_EPS_ENG_CDB_TLM, 0), "r");
		size = sizeof(ieps_enghk_data_cdb_t);
	}
	else if (tlmType==tlm_wod){
		fp = f_open(calculateFileName(date, END_FILENAME_WOD_TLM, 0), "r");
		size = sizeof(WOD_Telemetry_t);
	}
	else if (tlmType==tlm_solar){
		fp = f_open(calculateFileName(date, END_FILENAME_SOLAR_PANELS_TLM, 0), "r");
		size = sizeof(solar_tlm_t);
	}*/


	if (!fp)
	{
		printf("Unable to open file!");// TODO: log error in all printf in the file!
		return 1;
	}


	char buffer[(sizeof(current_time)+size) * NUM_ELEMENTS_READ_AT_ONCE];

	while(1)
	{
		int readElemnts = f_read(&buffer , sizeof(current_time)+size , NUM_ELEMENTS_READ_AT_ONCE, fp );

		if(!readElemnts) break;

		for (;readElemnts>0;readElemnts--){
			memcpy( &current_time, buffer + offset, sizeof(current_time) );
			printf("tlm time is:%d\n",current_time);
			offset += sizeof(current_time);
			if (tlmType==tlm_tx_revc){
				memcpy ( &txRevcData, buffer + offset, sizeof(txRevcData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(txRevcData);
			}
			else if (tlmType==tlm_tx){
				memcpy ( &txData, buffer + offset, sizeof(txData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(txData);
			}
			else if (tlmType==tlm_rx){
				memcpy ( &rxData, buffer + offset, sizeof(rxData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(rxData);
			}
			else if (tlmType==tlm_rx_revc){
				memcpy ( &rxData, buffer + offset, sizeof(rxRevcData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(rxRevcData);
			}
			else if (tlmType==tlm_rx_frame){
					memcpy ( &rxData, buffer + offset, sizeof(rxFrameData) );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
					offset += sizeof(rxFrameData);
			}
			else if (tlmType==tlm_wod){
				memcpy ( &wodData, buffer + offset, sizeof(wodData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(wodData);
			}
			else if (tlmType==tlm_eps_raw_mb){
				memcpy ( &engMbData, buffer + offset, sizeof(engMbData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(engMbData);
			}
			else if (tlmType==tlm_eps_eng_mb){
				memcpy ( &rxData, buffer + offset, sizeof(rxRevcData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(rxRevcData);
			}
			else if (tlmType==tlm_eps_raw_cdb){
					memcpy ( &rawCdbData, buffer + offset, sizeof(rawCdbData) );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
					offset += sizeof(rawCdbData);
			}
			else if (tlmType==tlm_eps_eng_cdb){
				memcpy ( &engCdbData, buffer + offset, sizeof(engCdbData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(engCdbData);
			}
			else if (tlmType==tlm_antenna){
				memcpy ( &antData, buffer + offset, sizeof(antData) );
				//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(antData);
			}
			else if (tlmType==tlm_solar){
							memcpy ( &solarData, buffer + offset, sizeof(solarData) );
							//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
							offset += sizeof(solarData);
						}


		}// end for loop...

	}

		/* close the file*/
		f_close (fp);
		return 0;
}



	int readTLMFiles(tlm_type_t tlmType, Time date, int numOfDays){
		for(int i = 0; i < numOfDays; i++){
			readTLMFile(tlmType, date, i);
		}

		return 0;
	}


	void DeInitializeFS( void )
	{
	}
