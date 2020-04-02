#include "FileSystemTestingDemo.h"
#include "GlobalStandards.h"
#include <hcc/api_fat.h>
#include <hal/Timing/Time.h>
#include <SubSystemModules/Housekepping/TelemetryFiles.h>
#include "utils.h"

Boolean TestlistFiels(){


	F_FIND find;
	if (!f_findfirst("*.*",&find))
	{
		do
		{
			printf("filename:%s",find.filename);

			if (find.attr&F_ATTR_DIR)
			{
				printf ("directory\n");
			}
			else
			{
				printf (" size %d\n",find.filesize);
			}
		} while (!f_findnext(&find));
	}
	printf("all file names printed\n");
	return TRUE;
}

Boolean TestReadWODFile(){
	Time theDay;
	theDay.year = 0;
	theDay.date = 1;
	theDay.month = 1;

	int numOfElementsSent = readTLMFile(tlm_wod,theDay,0);

	return TRUE;

}
Boolean TestLOGTLM(){

	// save current time
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	// set time to 2030/1/1
	Time_setUnixEpoch(1893456000);

	//delete the file
	Time theDay;
	theDay.year = 30;
	theDay.date = 1;
	theDay.month = 1;

	deleteTLMFile(tlm_log,theDay,0);

	// write some LOG elements in TLM file
	logData_t logData;
	strcpy(logData.msg, "test111");
	logData.error = 10; // just for testing...
	write2File(&logData,tlm_log);

	vTaskDelay(1500); // wait 1.5 sec, so we see a different timestamp in the file...

	strcpy(logData.msg, "test222");
	logData.error = 20; // just for testing...
	write2File(&logData,tlm_log);

	// read the data
	int numOfElementsSent = readTLMFile(tlm_log,theDay,0,2);

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}

Boolean TestWODTLM(){

	// save current time
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	// set time to 2030/1/1
	Time_setUnixEpoch(1893456000);

	//delete the file
	Time theDay;
	theDay.year = 30;
	theDay.date = 1;
	theDay.month = 1;

	deleteTLMFile(tlm_wod,theDay,0);

	// write some WOD elements in TLM file
	TelemetrySaveWOD();
	// read the data
	int numOfElementsSent = readTLMFile(tlm_wod,theDay,0,2);

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}



Boolean TestDeleteFiels(){

	// save current time
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	//delete 1st file
	Time theDay;
	theDay.year = 30;
	theDay.date = 1;
	theDay.month = 1;
	deleteTLMFile(tlm_log,theDay,0);

	//delete 2nd file
	theDay.date = 2;
	deleteTLMFile(tlm_log,theDay,0);

	//delete 3rd file
	theDay.date = 3;
	deleteTLMFile(tlm_log,theDay,0);


	// set time to 2030/1/1
	Time_setUnixEpoch(1893456000);

	printf ("writing 3 TLM files\n");
	// write some LOG elements in TLM file
	logData_t logData;
	strcpy(logData.msg, "test111");
	logData.error = 10;
	write2File(&logData,tlm_log);

	// set time to 2030/1/2
	Time_setUnixEpoch(1893542400);
	// write some LOG elements in TLM file
	write2File(&logData,tlm_log);

	// set time to 2030/1/3
	Time_setUnixEpoch(1893628800);
	// write some LOG elements in TLM file
	write2File(&logData,tlm_log);

	// lets check that we have 3 log files:
	int c = 0;
	F_FIND find;
	if (!f_findfirst("30010*.LOG",&find))
	{
		do
		{
			c++;
			printf("filename:%s",find.filename);

			if (find.attr&F_ATTR_DIR)
			{
				printf ("directory\n");
			}
			else
			{
				printf (" size %d\n",find.filesize);
			}
		} while (!f_findnext(&find));
	}

	if (c!=3){
		printf("*******  FAIL ***********");
	}

	printf ("deleting 2 TLM files\n");
	theDay.date = 1;
	deleteTLMFiles(tlm_log,theDay,2);

	// lets check that we have 1 log file left after we deleted 2 fiels
	c = 0;
	printf ("TLM files left:\n");
	if (!f_findfirst("30010*.LOG",&find))
	{
		do
		{
			c++;
			printf("filename:%s",find.filename);

			if (find.attr&F_ATTR_DIR)
			{
				printf ("directory\n");
			}
			else
			{
				printf (" size %d\n",find.filesize);
			}
		} while (!f_findnext(&find));
	}

	if (c!=1){
		printf("*******  FAIL ***********");
	}else{
		printf("*******  PASS ***********");
	}

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}


Boolean TestTRXVUTLM(){

	// save current time
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	// set time to 2030/1/1
	Time_setUnixEpoch(1893456000);

	//delete the file
	Time theDay;
	theDay.year = 30;
	theDay.date = 1;
	theDay.month = 1;

	deleteTLMFile(tlm_rx,theDay,0);
	deleteTLMFile(tlm_rx_frame,theDay,0);
	deleteTLMFile(tlm_tx,theDay,0);

	// write some WOD elements in TLM file
	TelemetrySaveTRXVU();
	// read the data
	printf("reading TX TLM\n");
	int numOfElementsSent = readTLMFile(tlm_tx,theDay,0,2);
	printf("reading RX TLM\n");
	numOfElementsSent = readTLMFile(tlm_rx,theDay,0,2);

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}


Boolean selectAndExecuteFSTest()
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) List files \n\r");
	printf("\t 2) Delete files \n\r");
	printf("\t 3) LOG TLM \n\r");
	printf("\t 4) WOD TLM \n\r");
	printf("\t 5) TRXVU TLM \n\r");

	unsigned int number_of_tests = 5;
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = TestlistFiels();
		break;
	case 2:
		offerMoreTests = TestDeleteFiels();
		break;
	case 3:
		offerMoreTests = TestLOGTLM();
		break;
	case 4:
		offerMoreTests = TestWODTLM();
		break;
	case 5:
		offerMoreTests = TestTRXVUTLM();
		break;
	default:
		break;
	}
	return offerMoreTests;
}

Boolean MainFileSystemTestBench()
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteFSTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return FALSE;
}
