/*
 * filesystem.c
 *
 *  Created on: 20 áîøõ 2019
 *      Author: Idan
 */


#include <satellite-subsystems/GomEPS.h>
#include <hal/Timing/Time.h>
#include <hcc/api_fat.h>
#include <hal/errors.h>
#include <hcc/api_hcc_mem.h>
#include <string.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <hal/Storage/FRAM.h>
#include <at91/utility/trace.h>
#include "TLM_management.h"
#include <stdlib.h>
#include <GlobalStandards.h>

#define SKIP_FILE_TIME_SEC 1000000
#define _SD_CARD 0
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
	return FS_SUCCSESS;
}

//only register the chain, files will create dynamically
FileSystemResult c_fileCreate(char* c_file_name,
		int size_of_element)
{
	return FS_SUCCSESS;
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

FileSystemResult c_fileWrite(char* c_file_name, void* element)
{
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
