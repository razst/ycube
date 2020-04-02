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
#include "SubSystemModules/Communication/TRXVU.h"


#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/isis_eps_driver.h>

#define SKIP_FILE_TIME_SEC 1000000
#define SD_CARD_DRIVER_PARMS 0
#define FIRST_TIME -1

#define NUM_ELEMENTS_READ_AT_ONCE 400 // TODO check if 400 is the right number !!!

// TODO: check the MAX size of the largest element type
static char buffer[(sizeof(logData_t)) * NUM_ELEMENTS_READ_AT_ONCE]; // buffer for data coming from SD (time+size of data struct)


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


void deleteTLMFiles(tlm_type_t tlmType, Time date, int numOfDays){
	for(int i = 0; i < numOfDays; i++){
		deleteTLMFile(tlmType,date,i);
	}

}


void deleteTLMFile(tlm_type_t tlmType, Time date, int days2Add){

	char endFileName [3] = {0};
	int size;
	getTlmTypeInfo(tlmType,endFileName,&size);

	char file_name[MAX_FILE_NAME_SIZE] = {0};
	calculateFileName(date,&file_name, endFileName, days2Add);

	f_delete(file_name);
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
void calculateFileName(Time curr_date,char* file_name, char* endFileName, int days2Add)
{
	/* initialize */
	struct tm t = { .tm_year = curr_date.year + 100, .tm_mon = curr_date.month - 1, .tm_mday = curr_date.date };
	/* modify */
	t.tm_mday += days2Add;
	mktime(&t);

	char buff[7];
	strftime(buff, sizeof buff, "%y%0m%0d", &t);
	snprintf(file_name, MAX_FILE_NAME_SIZE, "%s.%s", buff, endFileName);
}


void getTlmTypeInfo(tlm_type_t tlmType, char* endFileName, int* structSize){

	if (tlmType==tlm_tx){
		memcpy(endFileName,END_FILE_NAME_TX,sizeof(END_FILE_NAME_TX));
		*structSize = sizeof(ISIStrxvuTxTelemetry);
	}
	else if (tlmType==tlm_rx){
		memcpy(endFileName,END_FILE_NAME_RX,sizeof(END_FILE_NAME_RX));
		*structSize = sizeof(ISIStrxvuRxTelemetry);
	}
	else if (tlmType==tlm_rx_frame){
		memcpy(endFileName,END_FILE_NAME_RX_FRAME,sizeof(END_FILE_NAME_RX_FRAME));
		*structSize = sizeof(ISIStrxvuRxFrame);
	}
	else if (tlmType==tlm_antenna){
		memcpy(endFileName,END_FILE_NAME_ANTENNA,sizeof(END_FILE_NAME_ANTENNA));
		*structSize = sizeof(ISISantsTelemetry);
	}
	else if (tlmType==tlm_eps_raw_mb){
		memcpy(endFileName,END_FILENAME_EPS_RAW_MB_TLM,sizeof(END_FILENAME_EPS_RAW_MB_TLM));
		*structSize = sizeof(isis_eps__gethousekeepingraw__from_t);
	}
	else if (tlmType==tlm_eps_raw_cdb){
		memcpy(endFileName,END_FILENAME_EPS_RAW_CDB_TLM,sizeof(END_FILENAME_EPS_RAW_CDB_TLM));
		*structSize = sizeof(isis_eps__gethousekeepingrawincdb__from_t);
	}
	else if (tlmType==tlm_eps_eng_mb){
		memcpy(endFileName,END_FILENAME_EPS_ENG_MB_TLM,sizeof(END_FILENAME_EPS_ENG_MB_TLM));
		*structSize = sizeof(isis_eps__gethousekeepingeng__from_t);
	}
	else if (tlmType==tlm_eps_eng_cdb){
		memcpy(endFileName,END_FILENAME_EPS_ENG_CDB_TLM,sizeof(END_FILENAME_EPS_ENG_CDB_TLM));
		*structSize = sizeof(isis_eps__gethousekeepingengincdb__from_t);
	}
	else if (tlmType==tlm_wod){
		memcpy(endFileName,END_FILENAME_WOD_TLM,sizeof(END_FILENAME_WOD_TLM));
		*structSize = sizeof(WOD_Telemetry_t);
	}
	else if (tlmType==tlm_solar){
		memcpy(endFileName,END_FILENAME_SOLAR_PANELS_TLM,sizeof(END_FILENAME_SOLAR_PANELS_TLM));
		*structSize = sizeof(solar_tlm_t);
	}
	else if (tlmType==tlm_log){
		memcpy(endFileName,END_FILENAME_LOGS,sizeof(END_FILENAME_LOGS));
		*structSize = sizeof(logData_t);
	}

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
	char end_file_name[3] = {0};

	getTlmTypeInfo(tlmType,end_file_name,&size);
	calculateFileName(curr_date,&file_name,end_file_name , 0);
	fp = f_open(file_name, "a");

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


// data = timestampe+data (based on tlm type)
void printTLM(void* element, tlm_type_t tlmType){
#ifdef TESTING
	int offset = sizeof(int);

	// print the timestamp of the TLM element
	unsigned int element_time = *((unsigned int*)element);
	printf("TLM element: time:%u\n ",element_time);

	// print the data of the TLM element based on tlm_type
	if (tlmType==tlm_log){// TODO: switch
		logData_t data;
		memcpy(&data.error,element+offset,sizeof(int));
		offset += sizeof(data.error);

		memcpy(&data.msg,element+offset,MAX_LOG_STR);
		printf("error: %d\n ",data.error);
		printf("msg: %s\n",data.msg);
	}
	else if (tlmType==tlm_wod){
		WOD_Telemetry_t data;
		memcpy(&data.vbat,element+offset,sizeof(data.vbat));
		offset += sizeof(data.vbat);
		printf("vbat: %d\n ",data.vbat);

		memcpy(&data.volt_5V,element+offset,sizeof(data.volt_5V));
		offset += sizeof(data.volt_5V);
		printf("volt_5V: %d\n ",data.volt_5V);

		memcpy(&data.volt_3V3,element+offset,sizeof(data.volt_3V3));
		offset += sizeof(data.volt_3V3);
		printf("volt_3V3: %d\n ",data.volt_3V3);

		memcpy(&data.charging_power,element+offset,sizeof(data.charging_power));
		offset += sizeof(data.charging_power);
		printf("charging_power: %d\n ",data.charging_power);

		memcpy(&data.consumed_power,element+offset,sizeof(data.consumed_power));
		offset += sizeof(data.consumed_power);
		printf("consumed_power: %d\n ",data.consumed_power);

		memcpy(&data.electric_current,element+offset,sizeof(data.electric_current));
		offset += sizeof(data.electric_current);
		printf("electric_current: %d\n ",data.electric_current);

		memcpy(&data.current_3V3,element+offset,sizeof(data.current_3V3));
		offset += sizeof(data.current_3V3);
		printf("current_3V3: %d\n ",data.current_3V3);

		memcpy(&data.current_5V,element+offset,sizeof(data.current_5V));
		offset += sizeof(data.current_5V);
		printf("current_5V: %d\n ",data.current_5V);

		memcpy(&data.sat_time,element+offset,sizeof(data.sat_time));
		offset += sizeof(data.sat_time);
		printf("sat_time: %d\n ",data.sat_time);

		memcpy(&data.free_memory,element+offset,sizeof(data.free_memory));
		offset += sizeof(data.free_memory);
		printf("free_memory: %d\n ",data.free_memory);

		memcpy(&data.corrupt_bytes,element+offset,sizeof(data.corrupt_bytes));
		offset += sizeof(data.corrupt_bytes);
		printf("corrupt_bytes: %d\n ",data.corrupt_bytes);

		memcpy(&data.number_of_resets,element+offset,sizeof(data.number_of_resets));
		offset += sizeof(data.number_of_resets);
		printf("number_of_resets: %d\n ",data.number_of_resets);

		memcpy(&data.num_of_cmd_resets,element+offset,sizeof(data.num_of_cmd_resets));
		offset += sizeof(data.num_of_cmd_resets);
		printf("number_of_cmd_resets: %d\n ",data.num_of_cmd_resets);

	}else if (tlmType==tlm_tx){
		ISIStrxvuTxTelemetry data;
		offset += (sizeof(unsigned short) * 7);// skip 7 unsigned short fields
		memcpy(&data.fields.pa_temp,element+offset,sizeof(data.fields.pa_temp));
		offset += sizeof(data.fields.pa_temp);
		printf("pa_temp: %d\n ",data.fields.pa_temp);

		memcpy(&data.fields.board_temp,element+offset,sizeof(data.fields.board_temp));
		offset += sizeof(data.fields.board_temp);
		printf("board_temp: %d\n ",data.fields.board_temp);
	}
	else if (tlmType==tlm_rx){
		ISIStrxvuRxTelemetry data;
		offset += (sizeof(unsigned short) * 1);// skip 1 unsigned short fields
		memcpy(&data.fields.rx_rssi,element+offset,sizeof(data.fields.rx_rssi));
		offset += sizeof(data.fields.rx_rssi);
		printf("rx_rssi: %d\n ",data.fields.rx_rssi);

		memcpy(&data.fields.bus_volt,element+offset,sizeof(data.fields.bus_volt));
		offset += sizeof(data.fields.bus_volt);
		printf("bus_volt: %d\n ",data.fields.bus_volt);
	}

#endif
}



int readTLMFile(tlm_type_t tlmType, Time date, int numOfDays,int cmd_id){
	//TODO check for unsupported tlmType
	printf("reading from file...\n");

	unsigned int offset = 0;

	FILE * fp;
	int size=0;
	char file_name[MAX_FILE_NAME_SIZE] = {0};
	char end_file_name[3] = {0};

	getTlmTypeInfo(tlmType,end_file_name,&size);
	calculateFileName(date,&file_name,end_file_name , numOfDays);
	fp = f_open(file_name, "r");

	if (!fp)
	{
		printf("Unable to open file!");// TODO: log error in all printf in the file!
		return 1;
	}


	char element[(sizeof(int)+size)];// buffer for a single element that we will tx
	int numOfElementsSent=0;

	while(1)
	{
		int readElemnts = f_read(&buffer , sizeof(int)+size , NUM_ELEMENTS_READ_AT_ONCE, fp );

		offset = 0;
		if(!readElemnts) break;

		for (;readElemnts>0;readElemnts--){

			memcpy( &element, buffer + offset, sizeof(int) + size); // copy time+data
			offset += (size + sizeof(int));

			printTLM(&element,tlmType);

			sat_packet_t dump_tlm = { 0 };

			AssembleCommand((unsigned char*)element, sizeof(int)+size,
					trxvu_cmd_type,DUMP_SUBTYPE,
					cmd_id, &dump_tlm);

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

/*
	int readTLMFileTimeRange(tlm_type_t tlmType,time_t from_time,time_t to_time, Time date){
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
		logData_t logsData;

		FILE * fp;
		int size=0;


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
		}
		if (tlmType==tlm_log){
			fp = f_open(calculateFileName(date, END_FILENAME_LOGS, 0), "r"); // TODO: check if we can pass char* to f_open  not sure
			size = sizeof(logData_t);
		}

		if (!fp)
		{
			printf("Unable to open file!");
			return 1;
		}

		while (f_read(&current_time , sizeof(current_time) , 1, fp ) == 1){
			printf("tlm time is:%d\n",current_time);
			if (tlmType==tlm_tx_revc){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&txRevcData , sizeof(txRevcData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(txRevcData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_tx){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&txData , sizeof(txData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(txData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_rx){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&rxData , sizeof(rxData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(rxData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_rx_revc){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&rxRevcData , sizeof(rxRevcData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(rxRevcData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_rx_frame){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&rxFrameData , sizeof(rxFrameData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(rxFrameData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_wod){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&wodData , sizeof(wodData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(wodData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_eps_raw_mb){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&rawMbData , sizeof(rawMbData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(rawMbData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_eps_eng_mb){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&engMbData , sizeof(engMbData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(engMbData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_eps_eng_cdb){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&engCdbData , sizeof(engCdbData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(engCdbData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_eps_raw_cdb){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&rawCdbData , sizeof(rawCdbData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(rawCdbData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_solar){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&solarData , sizeof(solarData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(solarData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_antenna){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&antData , sizeof(antData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(antData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}
			} else if (tlmType==tlm_log){
				if (current_time>=from_time && current_time<=to_time){
					f_read(&antData , sizeof(antData) , 1, fp );
					//				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				}else if (current_time<=to_time){
					f_seek (fp, sizeof(antData), SEEK_CUR);
				}else{
					break; // we passed over the date we needed, no need to look anymore...
				}

			}
		}


		f_close (fp);
		return 0;
	}

 */
void DeInitializeFS( void )
{
}
