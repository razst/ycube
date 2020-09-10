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
#define SD_CARD_DRIVER_PRI 0
#define SD_CARD_DRIVER_SEC 1
#define FIRST_TIME -1

static char buffer[ MAX_COMMAND_DATA_LENGTH * NUM_ELEMENTS_READ_AT_ONCE]; // buffer for data coming from SD (time+size of data struct)

int CMD_getInfoImage(sat_packet_t *cmd){
	FILE * f_source;
	unsigned short numChunk;
	char imageFileName[5];
	char imageType;
	memcpy(&imageType, &cmd->data, sizeof(char));

	if(imageType < 6){
	  sprintf(imageFileName, "%d.JPG",imageType);
	  f_source = f_open(imageFileName, "r");
	}else if(imageType == 6){
		// TODO: heat map
	}else{
		return INVALID_IMG_TYPE;
	}

	if (!f_source)
	{
		//printf("Unable to open file\n");
		return -1;
	}

	f_seek (f_source, 0 , SEEK_END);
	long size = f_tell (f_source);
	f_rewind (f_source);
	f_close(f_source);
	numChunk = size / IMG_CHUNK_SIZE;
	if((size % IMG_CHUNK_SIZE) != 0){
		numChunk++;
	}
	imageInfo_t data;
	data.imageID = imageType; // TODO: change for image type 6 (heat map)
	data.imageType = imageType;
	memcpy(&data.numberChunks,&numChunk, sizeof(numChunk));
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &data,
			sizeof(data));

}


int CMD_getDataImage(sat_packet_t *cmd){
//	short chunk_list[] = ;

	FILE * f_source;
	char image[5];
	short imageID;
	memcpy(&imageID, &cmd->data, sizeof(short));
	sprintf(image, "%d.jpg", imageID);
		f_source = f_open(image, "r");
		if (!f_source)
		{
			//printf("Unable to open file\n");
			return -1;
		}

		 // calculating source file size
		f_seek (f_source, 0 , SEEK_END);
		long size = f_tell (f_source);
		f_rewind (f_source);
		//printf("source file size:%d. chunk size:%d\n",size,IMG_CHUNK_SIZE);



		char buffer[IMG_CHUNK_SIZE]={0};

		unsigned short chunk = 0;
		imageData_t data;

		while (size>0){
			chunk++;
			size = size - IMG_CHUNK_SIZE;

			int readSize = f_read(buffer,1,IMG_CHUNK_SIZE,f_source);

			if (readSize==0){
		        f_puts ("Reading error",stderr);
		    	fclose(f_source);
		        return -1;
			}
			memcpy(&data.chunkID,&chunk,sizeof(data.chunkID));
			memcpy(&data.data,&buffer, IMG_CHUNK_SIZE);

			//printf("about to transmit chunk:%d\n",chunk);
			TransmitDataAsSPL_Packet(cmd, (unsigned char*) &data,
						sizeof(data));

		}
}


void delete_allTMFilesFromSD()
{
	// TODO make sure we don't delete the image file

	int c = 100;
	F_FIND find;
	if (!f_findfirst("A:/*.*",&find))
	{
		do
		{
			f_delete(find.filename);
			c++;
			if (c>=100){
				c=0;
				unsigned int curr_time;
				Time_getUnixEpoch(&curr_time);
				//printf("time is: %d",curr_time);
			}
		} while (!f_findnext(&find));
	}
}


int deleteTLMFiles(tlm_type_t tlmType, Time date, int numOfDays){
	int deletedFiles = 0;
	for(int i = 0; i < numOfDays; i++){
		if (deleteTLMFile(tlmType,date,i) == F_NO_ERROR){
			deletedFiles++;
		}
	}
	return deletedFiles;
}


int deleteTLMFile(tlm_type_t tlmType, Time date, int days2Add){

	char endFileName [3] = {0};
	int size;
	getTlmTypeInfo(tlmType,endFileName,&size);

	char file_name[MAX_FILE_NAME_SIZE] = {0};
	calculateFileName(date,&file_name, endFileName, days2Add);

	return f_delete(file_name);
}

FileSystemResult InitializeFS(Boolean first_time)
{

	// in FS init we don't want to use a log file !
	// Initialize the memory for the FS
	int err = hcc_mem_init();
	if (err != E_NO_SS_ERR){
		//printf("hcc_mem_init error:",err);
	}

	// Initialize the FS
	err = fs_init();
	if (err != E_NO_SS_ERR){
		//printf("fs_init error:",err);
	}

	fs_start();

	// Tell the OS (freeRTOS) about our FS
	err = f_enterFS();
	if (err != E_NO_SS_ERR){
		//printf("f_enterFS error:",err);
	}

	// Initialize the volume of SD card 0 (A)
	// TODO should we also init the volume of SD card 1 (B)???
	// TODO: why drive 1 is not working when we test it?
	err=f_initvolume( 0, atmel_mcipdc_initfunc, SD_CARD_DRIVER_PRI );
	if (err != E_NO_SS_ERR){
		// erro init SD 0 so de-itnit and init SD 1
		//printf("f_initvolume primary error:%d\n",err);
		DeInitializeFS(SD_CARD_DRIVER_PRI);
		hcc_mem_init();
		fs_init();
		f_enterFS();
		err=f_initvolume( 0, atmel_mcipdc_initfunc, SD_CARD_DRIVER_SEC );
		if (err != E_NO_SS_ERR){
			//printf("f_initvolume secondary error:%d\n",err);
		}
	}

	//In the first time the SD on. if there is file on the SD delete it.
	if(first_time) delete_allTMFilesFromSD();

	return FS_SUCCSESS;
}



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
	else if (tlmType==tlm_eps_raw_mb_NOT_USED){
		memcpy(endFileName,END_FILENAME_EPS_RAW_MB_TLM,sizeof(END_FILENAME_EPS_RAW_MB_TLM));
		*structSize = sizeof(isis_eps__gethousekeepingraw__from_t);
	}
	else if (tlmType==tlm_eps_raw_cdb_NOT_USED){
		memcpy(endFileName,END_FILENAME_EPS_RAW_CDB_TLM,sizeof(END_FILENAME_EPS_RAW_CDB_TLM));
		*structSize = sizeof(isis_eps__gethousekeepingrawincdb__from_t);
	}
	else if (tlmType==tlm_eps){
		memcpy(endFileName,END_FILENAME_EPS_TLM,sizeof(END_FILENAME_EPS_TLM));
		*structSize = sizeof(isis_eps__gethousekeepingeng__from_t);
	}
	else if (tlmType==tlm_eps_eng_cdb_NOT_USED){
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
	//printf("writing tlm: %d to SD\n",tlmType);

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

	int err = f_getlasterror();

	if (!fp)
	{
		//printf("Unable to open file %s, f_open error=%d\n",file_name, err);
		return -1;
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
	//printf("TLM element: time:%u\n ",element_time);

	// print the data of the TLM element based on tlm_type
	if (tlmType==tlm_log){
		logData_t data;
		memcpy(&data.error,element+offset,sizeof(int));
		offset += sizeof(data.error);

		memcpy(&data.msg,element+offset,MAX_LOG_STR);
		//printf("error: %d\n ",data.error);
		//printf("msg: %s\n",data.msg);
	}
	else if (tlmType==tlm_wod){
		WOD_Telemetry_t data;
		memcpy(&data.vbat,element+offset,sizeof(data.vbat));
		offset += sizeof(data.vbat);
		//printf("vbat: %d\n ",data.vbat);

		memcpy(&data.volt_5V,element+offset,sizeof(data.volt_5V));
		offset += sizeof(data.volt_5V);
		//printf("volt_5V: %d\n ",data.volt_5V);

		memcpy(&data.volt_3V3,element+offset,sizeof(data.volt_3V3));
		offset += sizeof(data.volt_3V3);
		//printf("volt_3V3: %d\n ",data.volt_3V3);

		memcpy(&data.charging_power,element+offset,sizeof(data.charging_power));
		offset += sizeof(data.charging_power);
		//printf("charging_power: %d\n ",data.charging_power);

		memcpy(&data.consumed_power,element+offset,sizeof(data.consumed_power));
		offset += sizeof(data.consumed_power);
		//printf("consumed_power: %d\n ",data.consumed_power);

		memcpy(&data.electric_current,element+offset,sizeof(data.electric_current));
		offset += sizeof(data.electric_current);
		//printf("electric_current: %d\n ",data.electric_current);

		memcpy(&data.current_3V3,element+offset,sizeof(data.current_3V3));
		offset += sizeof(data.current_3V3);
		//printf("current_3V3: %d\n ",data.current_3V3);

		memcpy(&data.current_5V,element+offset,sizeof(data.current_5V));
		offset += sizeof(data.current_5V);
		//printf("current_5V: %d\n ",data.current_5V);

		memcpy(&data.sat_time,element+offset,sizeof(data.sat_time));
		offset += sizeof(data.sat_time);
		//printf("sat_time: %d\n ",data.sat_time);

		memcpy(&data.free_memory,element+offset,sizeof(data.free_memory));
		offset += sizeof(data.free_memory);
		//printf("free_memory: %d\n ",data.free_memory);

		memcpy(&data.corrupt_bytes,element+offset,sizeof(data.corrupt_bytes));
		offset += sizeof(data.corrupt_bytes);
		//printf("corrupt_bytes: %d\n ",data.corrupt_bytes);

		memcpy(&data.number_of_resets,element+offset,sizeof(data.number_of_resets));
		offset += sizeof(data.number_of_resets);
		//printf("number_of_resets: %d\n ",data.number_of_resets);

		memcpy(&data.num_of_cmd_resets,element+offset,sizeof(data.num_of_cmd_resets));
		offset += sizeof(data.num_of_cmd_resets);
		//printf("number_of_cmd_resets: %d\n ",data.num_of_cmd_resets);

	}else if (tlmType==tlm_tx){
		ISIStrxvuTxTelemetry data;
		offset += (sizeof(unsigned short) * 7);// skip 7 unsigned short fields
		memcpy(&data.fields.pa_temp,element+offset,sizeof(data.fields.pa_temp));
		offset += sizeof(data.fields.pa_temp);
		//printf("pa_temp: %d\n ",data.fields.pa_temp);

		memcpy(&data.fields.board_temp,element+offset,sizeof(data.fields.board_temp));
		offset += sizeof(data.fields.board_temp);
		//printf("board_temp: %d\n ",data.fields.board_temp);
	}
	else if (tlmType==tlm_rx){
		ISIStrxvuRxTelemetry data;
		offset += (sizeof(unsigned short) * 1);// skip 1 unsigned short fields
		memcpy(&data.fields.rx_rssi,element+offset,sizeof(data.fields.rx_rssi));
		offset += sizeof(data.fields.rx_rssi);
		//printf("rx_rssi: %d\n ",data.fields.rx_rssi);

		memcpy(&data.fields.bus_volt,element+offset,sizeof(data.fields.bus_volt));
		offset += sizeof(data.fields.bus_volt);
		//printf("bus_volt: %d\n ",data.fields.bus_volt);
	}
	else if (tlmType==tlm_eps_raw_cdb){
		isis_eps__gethousekeepingrawincdb__from_t data;
			offset += (sizeof(isis_eps__replyheader_t));// skip 1 unsigned short fields
			offset += (sizeof(uint8_t));// skip 1 unsigned short fields
			memcpy(&data.fields.volt_brdsup ,element+offset,sizeof(data.fields.volt_brdsup));
			offset += sizeof(data.fields.volt_brdsup);
			//printf("volt_brdsup: %d\n ",data.fields.volt_brdsup);

			memcpy(&data.fields.temp,element+offset,sizeof(data.fields.temp));
			offset += sizeof(data.fields.temp);
			//printf("temp: %d\n ",data.fields.temp);
		}
#endif
}



int readTLMFile(tlm_type_t tlmType, Time date, int numOfDays,int cmd_id, int resolution){

	unsigned int offset = 0;

	FILE * fp;
	int size=0;
	char file_name[MAX_FILE_NAME_SIZE] = {0};
	char end_file_name[3] = {0};

	getTlmTypeInfo(tlmType,end_file_name,&size);
	calculateFileName(date,&file_name,end_file_name , numOfDays);
	//printf("reading from file %s...\n",file_name);
	fp = f_open(file_name, "r");


	int err = f_getlasterror();

	if (!fp)
	{
		//printf("Unable to open file %s, f_open error=%d\n",file_name, err);
		return -1;
	}



	char element[(sizeof(int)+size)];// buffer for a single element that we will tx
	int numOfElementsSent=0;
	time_unix currTime = 0;
	time_unix lastSentTime = 0;


	while(1)
	{
		int readElemnts = f_read(&buffer , sizeof(int)+size , NUM_ELEMENTS_READ_AT_ONCE, fp );

		offset = 0;
		if(!readElemnts) break;

		for (;readElemnts>0;readElemnts--){

			memcpy( &element, buffer + offset, sizeof(int) + size); // copy time+data
			offset += (size + sizeof(int));

			// get the time of the current element and check if we need to send it or not based on the resolution
			memcpy(&currTime, &element, sizeof(int));
			if (currTime - lastSentTime >= resolution){
				// time to send the data
				lastSentTime = currTime;
				printTLM(&element,tlmType);

				sat_packet_t dump_tlm = { 0 };

				AssembleCommand((unsigned char*)element, sizeof(int)+size,
						dump_type,tlmType,
						cmd_id, &dump_tlm);

				TransmitSplPacket(&dump_tlm, NULL);
				numOfElementsSent++;
				if(CheckDumpAbort()){
					stopDump = TRUE;
					break;
				}
			}

		}// end for loop...
		if(stopDump){
			break;
		}
	}// while (1) loop...

	/* close the file*/
	f_close (fp);
	return numOfElementsSent;
}



int readTLMFiles(tlm_type_t tlmType, Time date, int numOfDays,int cmd_id,int resolution){
	stopDump = FALSE;
	int totalReads=0;
	int elemntsRead=0;
	for(int i = 0; i < numOfDays; i++){
		elemntsRead=readTLMFile(tlmType, date, i,cmd_id,resolution);
		totalReads+=(elemntsRead>0) ? elemntsRead : 0;
		if(stopDump){
			break;
		}
		vTaskDelay(100);
	}

	return totalReads;
}


int readTLMFileTimeRange(tlm_type_t tlmType,time_t from_time,time_t to_time, int cmd_id, int resolution){

	if (from_time >= to_time)
		return E_INVALID_PARAMETERS;

	Time date;
	timeU2time(from_time,&date);

	//printf("reading from file...\n");

	FILE * fp;
	int size=0;
	char file_name[MAX_FILE_NAME_SIZE] = {0};
	char end_file_name[3] = {0};

	getTlmTypeInfo(tlmType,end_file_name,&size);
	calculateFileName(date,&file_name,end_file_name ,0);
	fp = f_open(file_name, "r");


	int err = f_getlasterror();

	if (!fp)
	{
		//printf("Unable to open file %s, f_open error=%d\n",file_name, err);
		return -1;
	}



	int numOfElementsSent = 0;
	char element[(sizeof(int)+size)];// buffer for a single element that we will tx
	time_unix current_time=0;
	time_unix lastSentTime=0;
	while (f_read(&element , sizeof(int)+size , 1, fp ) == 1){
		// get the time of the current element and check if we need to send it or not based on the resolution
		memcpy(&current_time, &element, sizeof(int));
		////printf("tlm time is:%d\n",current_time);
		if (current_time>=from_time && current_time<=to_time && ((current_time - lastSentTime) >= resolution)){
			lastSentTime = current_time;

			printTLM(&element,tlmType);
			sat_packet_t dump_tlm = { 0 };

			AssembleCommand((unsigned char*)element, sizeof(int)+size,
					dump_type,tlmType,
					cmd_id, &dump_tlm);


			TransmitSplPacket(&dump_tlm, NULL);
			numOfElementsSent++;
			if(CheckDumpAbort()){
				break;
			}


		//}else if (current_time<=to_time){
		//	f_seek (fp, size, SEEK_CUR);
		}if (current_time>to_time){
			break; // we passed over the date we needed, no need to look anymore...
		}
	}


	f_close (fp);
	return numOfElementsSent;
}


void DeInitializeFS( int sd_card )
{
	//printf("deinitializig file system. SD card: %d \n",sd_card);
	int err = f_delvolume( sd_card ); /* delete the volID */

	//printf("volume deleted\n");

	if(err != 0)
	{
		//printf("f_delvolume err %d\n", err);
	}

	f_releaseFS(); /* release this task from the filesystem */

	//printf("FS released\n");

	err = fs_delete(); /* delete the filesystem */

	//printf("FS deleted\n");

	if(err != 0)
	{
		//printf("fs_delete err , %d\n", err);
	}
	err = hcc_mem_delete(); /* free the memory used by the filesystem */

	//printf("FS mem freed\n");

	if(err != 0)
	{
		//printf("hcc_mem_delete err , %d\n", err);
	}
	//printf("deinitializig file system done \n");

}
