/*
 * filesystem.c
 *
 *  Created on: 20 áîøõ 2019
 *      Author: Idan
 */


//#include <at91/utility/trace.h>
//#include <GlobalStandards.h>
//#include <hal/errors.h>
//#include <hal/Storage/FRAM.h>
#include <hal/Timing/Time.h>
#include <hcc/api_fat.h>
#include <hcc/api_hcc_mem.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <stdio.h>
//#include <SubSystemModules/Communication/SPL.h>
#include <TLM_management.h>
#include <utils.h>
#include <string.h>

#define SKIP_FILE_TIME_SEC 1000000
#define SD_CARD_DRIVER_PARMS 0
#define FIRST_TIME -1
#define FILE_NAME_WITH_INDEX_SIZE MAX_F_FILE_NAME_SIZE+sizeof(int)*2

//struct for filesystem info
typedef struct
{
	int num_of_files;
} FS;
//TODO remove all 'PLZNORESTART' from code!!
#define PLZNORESTART() gom_eps_hk_basic_t myEpsTelemetry_hk_basic;	(GomEpsGetHkData_basic(0, &myEpsTelemetry_hk_basic)); //todo delete

//struct for chain file info
typedef struct
{
	int size_of_element;
	char name[FILE_NAME_WITH_INDEX_SIZE];
	unsigned int creation_time;
	unsigned int last_time_modified;
	int num_of_files;

} C_FILE;
#define C_FILES_BASE_ADDR (FSFRAM+sizeof(FS))


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
// return -1 for FRAM fail
static int getNumOfFilesInFS()
{
	FS fs;
	return fs.num_of_files;
}
//return -1 on fail
static int setNumOfFilesInFS(int new_num_of_files)
{
	return 0;
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

//only register the chain, files will create dynamically
FileSystemResult c_fileCreate(char* c_file_name,
		int size_of_element)
{
	return FS_SUCCSESS;
}

char* calculateFileName(char year, char month, char day)
{
	char file_name[11];

	if(day < 10 && month < 10){
		snprintf(file_name, sizeof file_name, "%i0%i0%i.%s", year, month, day, FS_FILE_ENDING);
	} else if (day < 10) {
		snprintf(file_name, sizeof file_name, "%i%i0%i.%s", year, month, day, FS_FILE_ENDING);
	} else if (month < 10){
		snprintf(file_name, sizeof file_name, "%i0%i%i.%s", year, month, day, FS_FILE_ENDING);
	} else {
		snprintf(file_name, sizeof file_name, "%i%i%i.%s", year, month, day, FS_FILE_ENDING);
	}

	return &file_name;
}

char* calculateData2Write2File(char data, int size)
{
	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);
	char data2Write2File[size + 32];
	//max!
	snprintf(data2Write2File, sizeof data2Write2File, "%i;%s" , curr_time , data);
	return &data2Write2File;
}

static void write2File(char* data, int size)
{
	Time curr_time;
	Time_get(&curr_time);

	F_FILE *file;
	file = f_open(calculateFileName(curr_time.year, curr_time.month, curr_time.date), "a");
	f_write(calculateData2Write2File(data, size), 1, size, file);
	f_close(file);
}

char* getFileData(char year, char month, char day)
{
	F_FILE *file = f_open(calculateFileName(year, month, day),"r");
	long size = f_filelength(calculateFileName(year, month, day));

	if (!file)
	{
		// log...
		char error[40] = "Error, there is no log file to this day";
		return &error;
	}

	char data[size];
	if (f_read(data,1,size,file)!=size)
	{
		// log...
		char errorAndData[size + 62];
		snprintf(errorAndData, sizeof errorAndData, "%s%s", "Error, can't get all log data! \nbut this is what I read:\n\n", data);
		f_close(file);
		return &errorAndData;
	}

	f_close(file);
	return &data;
}

char* getFilesData(Time fileDay, time_unix fromTime, time_unix toTime)
{
	F_FILE *file = f_open(calculateFileName(fileDay.year, fileDay.month, fileDay.day),"r");
	long size = f_filelength(calculateFileName(fileDay.year, fileDay.month, fileDay.day));

	if (!file)
	{
		// log...
		char error[40] = "Error, there is no log file to this day";
		return &error;
	}

	char data[size];
	if (f_read(data,1,size,file)!=size)
	{
		// log...
		char errorAndData[size + 62];
		snprintf(errorAndData, sizeof errorAndData, "%s%s", "Error, can't get all log data! \nbut this is what I read:\n\n", data);
		f_close(file);
		return &errorAndData;
	}

	f_close(file);

	  char file_data[] ="1;data\n2;data\n3;data";
	  char line_date[18], line_data[100];

	  printf("%lu", sizeof(file_data));

	  //while EOF

	  for(int i = 0; i < sizeof(file_data); i++){
	    for(; i < sizeof(file_data); i++){ //time (;)
	      if(file_data[i] == ";")
	        break;
	      snprintf(line_date, sizeof line_date, "%s%c", line_date, file_data[i]);
	    }

	    for(; i < sizeof(file_data); i++){ //time (;)
	      if(file_data[i] == "\n")
	        break;
	      snprintf(line_data, sizeof line_data, "%s%c", line_data, file_data[i]);
	    }

	    // if() //add to return data!
	  }


	return &data;
}

//write element with timestamp to file
static void writewithEpochtime(F_FILE* file, byte* data, int size,unsigned int time)
{

}
// get C_FILE struct from FRAM by name
static Boolean get_C_FILE_struct(char* name,C_FILE* c_file,unsigned int *address)
{
	return FALSE;
}
//calculate index of file in chain file by time
static int getFileIndex(unsigned int creation_time, unsigned int current_time)
{
	return 0;
}
//write to curr_file_name
void get_file_name_by_index(char* c_file_name,int index,char* curr_file_name)
{
}
FileSystemResult c_fileReset(char* c_file_name)
{
	return FS_SUCCSESS;
}

FileSystemResult c_fileWrite(void* element)
{
	Time t;
	Time_get(&t);

	return FS_SUCCSESS;
}
FileSystemResult fileWrite(char* file_name, void* element,int size)
{
	return FS_SUCCSESS;
}

static FileSystemResult deleteElementsFromFile(char* file_name,unsigned long from_time,
		unsigned long to_time,int full_element_size)
{
	return FS_SUCCSESS;
}
FileSystemResult c_fileDeleteElements(char* c_file_name, time_unix from_time,
		time_unix to_time)
{
	return FS_SUCCSESS;
}
FileSystemResult fileRead(char* c_file_name,byte* buffer, int size_of_buffer,
		time_unix from_time, time_unix to_time, int* read, int element_size)
{
	return FS_SUCCSESS;
}
FileSystemResult c_fileRead(char* c_file_name,byte* buffer, int size_of_buffer,
		time_unix from_time, time_unix to_time, int* read,time_unix* last_read_time)
{
	return FS_SUCCSESS;
}
void print_file(char* c_file_name)
{
}

void DeInitializeFS( void )
{
}
typedef struct{
	int a;
	int b;
}TestStruct ;
