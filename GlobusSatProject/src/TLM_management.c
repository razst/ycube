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
#include <time.h>

#define SKIP_FILE_TIME_SEC 1000000
#define SD_CARD_DRIVER_PARMS 0
#define FIRST_TIME -1
#define FILE_NAME_WITH_INDEX_SIZE MAX_F_FILE_NAME_SIZE+sizeof(int)*2

#define NUM_ELEMENTS_READ_AT_ONCE 400
typedef enum {EPS,TRXVU,LOG} tlm_t;

typedef struct  {
	float vBat;
	int satState;
} EPS_TLM;

typedef struct  {
	int bytesTX;
	int bytesRX;
	int txBaud;
} TRXVU_TLM;


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


//TODO when we get the Sat, check if we get 01 or 1 for the day and update the code
char* calculateFileName(Time curr_date, char endFileName, int dayBack)
{
	/* initialize */
	struct tm t = { .tm_year = curr_date.year + 100, .tm_mon = curr_date.month - 1, .tm_mday = curr_date.date };
	/* modify */
	t.tm_mday += dayBack;
	mktime(&t);

	char file_name[11], buff[7];
	strftime(buff, sizeof buff, "%y%0m%0d", &t);
	snprintf(file_name, sizeof file_name, "%s.%s", buff, endFileName);

	return &file_name;
}

int write2File(void* data, tlm_t tlmType){
	// TODO what happens if there was an error writing to SD, the whole file will be corrupted for us.
	printf("writing 2 file...\n");

	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);

	Time curr_date;
	Time_get(&curr_date);

	int size;
	F_FILE *fp;

	/* open the file for writing in append mode*/
	if (tlmType==EPS){
		fp = f_open(calculateFileName(curr_date, "eps", 0), "a");
		size = sizeof(EPS_TLM);
	}else if (tlmType==TRXVU){
		fp = f_open(calculateFileName(curr_date, "trx", 0), "a");
		size = sizeof(TRXVU_TLM);
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

int readTLMFile(Time curr_date, tlm_t tlmType, int numOfDays){
	//TODO check for unsupported tlmType
	printf("reading from file...\n");
	unsigned int current_time;
	EPS_TLM epsData;
	TRXVU_TLM trxvuData;
	unsigned int offset = 0;

	FILE * fp;

	/* open the file for writing in append mode*/
	if (tlmType==EPS){
		fp = f_open(calculateFileName(curr_date, "eps", numOfDays), "r");
	}else if (tlmType==TRXVU){
		fp = f_open(calculateFileName(curr_date, "trx", numOfDays), "r");
	}

	if (!fp)
	{
		printf("Unable to open file!");
		return 1;
	}

	char buffer[(sizeof(current_time)+sizeof(epsData))*NUM_ELEMENTS_READ_AT_ONCE];

	while(1)
	{
		int readElemnts = f_read(&buffer , sizeof(current_time)+sizeof(epsData) , NUM_ELEMENTS_READ_AT_ONCE, fp );

		if(!readElemnts) break;

		// TODO what if readElemnts==0...
		f_close (fp);
		for (;readElemnts>0;readElemnts--){
			memcpy( &current_time, buffer + offset, sizeof(current_time) );
			printf("tlm time is:%d\n",current_time);
			offset += sizeof(current_time);
			if (tlmType==EPS){
				memcpy ( &epsData, buffer + offset, sizeof(epsData) );
				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
				offset += sizeof(epsData);
			}else if (tlmType==TRXVU){
				// TODO ...
			}
		}
	}


	/* close the file*/
	f_close (fp);
	return 0;
}

int readTLMFiles(Time curr_date, tlm_t tlmType, int numOfDays){
	for(int i = 0; i < numOfDays; i++){
		readTLMFile(curr_date, tlmType, i);
	}

	return 0;
}

int readTLMFileTimeRange(tlm_t tlmType,time_t from_time,time_t to_time, Time curr_date){
	//TODO check for unsupported tlmType
	printf("reading from file...\n");
	unsigned int current_time;
	EPS_TLM epsData;
	TRXVU_TLM trxvuData;

	FILE * fp;

	/* open the file for writing in append mode*/
	if (tlmType==EPS){
		fp = f_open(calculateFileName(curr_date,"eps", 0), "rb");
	}else if (tlmType==TRXVU){
		fp = f_open(calculateFileName(curr_date,"trx", 0), "rb");
	}

	if (!fp)
	{
		printf("Unable to open file!");
		return 1;
	}

	while (f_read(&current_time , sizeof(current_time) , 1, fp ) == 1){
		printf("tlm time is:%d\n",current_time);
		if (tlmType==EPS){
			if (current_time>=from_time && current_time<=to_time){
				f_read(&epsData , sizeof(epsData) , 1, fp );
				printf("EPS data = %d,%f\n",epsData.satState,epsData.vBat);
			}else if (current_time<=to_time){
				f_seek (fp, sizeof(epsData), SEEK_CUR);
			}else{
				break; // we passed over the date we needed, no need to look anymore...
			}
		} else if (tlmType==TRXVU){
			// TODO check from_time && to_time like EPS
			f_read(&trxvuData , sizeof(trxvuData) , 1, fp );
			printf("TRXVO data = %d,%f\n",trxvuData.bytesRX,trxvuData.bytesTX);
		}
	}

	if (f_eof(fp))
	{
		// hit end of file
	}
	else
	{

	}

	/* close the file*/
	f_close (fp);
	return 0;
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
