#include "FileSystemTestingDemo.h"
#include "GlobalStandards.h"
#include <hcc/api_fat.h>
#include <hal/Timing/Time.h>
#include <SubSystemModules/Housekepping/TelemetryFiles.h>
#include "utils.h"
#include "SubSystemModules/Maintenance/Maintenance.h"

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

	int numOfElementsSent = readTLMFile(tlm_wod,theDay,0,3,0);

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
	int numOfElementsSent = readTLMFile(tlm_log,theDay,0,2,0);

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}

Boolean TestEPSTLM(){

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

	deleteTLMFile(tlm_eps_raw_cdb,theDay,0);

	// write some LOG elements in TLM file
	TelemetrySaveEPS();
	// read the data
	int numOfElementsSent = readTLMFile(tlm_eps_raw_cdb,theDay,0,2,0);

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}
Boolean TestReadTimeRangeTLMRes(){
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

	for(int i=0;i<10;i++){
		logData_t logData;
		strcpy(logData.msg, "test111");
		logData.error = i; // just for testing...
		write2File(&logData,tlm_log);

		vTaskDelay(1500); // wait 1.5 sec, so we see a different timestamp in the file...
	}

	printf("***** reading all data with resolution 0 ***** \n");
	int numOfElementsSent0 = readTLMFile(tlm_log,theDay,0,2,0);
	printf("***** reading all data with resolution 2 sec ***** \n");
	int numOfElementsSent2 = readTLMFile(tlm_log,theDay,0,2,2);

	if (numOfElementsSent0 == 10 && numOfElementsSent2 == 6){
		printf("*******  PASS (1 of 2) ***********\n");
	}else{
		printf("*******  FAIL (1 of 2) ***********\n");
	}

	// set the time back
	Time_setUnixEpoch(current_time);

	printf("***** reading data with range and resolution 0 ***** \n");
	numOfElementsSent0 = readTLMFileTimeRange( tlm_log ,1893456003,1893456009,3,0);
	printf("***** reading data with range and resolution 2 ***** \n");
	numOfElementsSent2 = readTLMFileTimeRange( tlm_log ,1893456003,1893456009,3,2);
	if (numOfElementsSent0 == 5 && numOfElementsSent2 == 3){
		printf("*******  PASS (2 of 2) ***********\n");
	}else{
		printf("*******  FAIL (2 of 2) ***********\n");
	}

	return TRUE;

}


Boolean TestReadTimeRangeTLM(){

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

	for(int i=0;i<10;i++){
		logData_t logData;
		strcpy(logData.msg, "test111");
		logData.error = i; // just for testing...
		write2File(&logData,tlm_log);

		vTaskDelay(1500); // wait 1.5 sec, so we see a different timestamp in the file...
	}


	// set the time back
	Time_setUnixEpoch(current_time);

	printf("***** reading full day data ***** \n");
	int numOfElementsSent0 = readTLMFile(tlm_log,theDay,0,2,0);
	printf("***** reading data with range ***** \n");
	int numOfElementsSent2 = readTLMFileTimeRange( tlm_log ,1893456003,1893456009,3,0);
	if (numOfElementsSent0 == 10 && numOfElementsSent2 == 5){
		printf("*******  PASS ***********\n");
	}else{
		printf("*******  FAIL  ***********\n");
	}

	return TRUE;

}



Boolean GenerateWODTLM(){

	// save current time
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	// set time to 2040/1/1
	Time_setUnixEpoch(1924992000);

	//delete the file
	Time theDay;
	theDay.year = 31;
	theDay.date = 1;
	theDay.month = 1;

	deleteTLMFile(tlm_wod,theDay,0);

	// write some WOD elements in TLM file
	TelemetrySaveWOD();
	vTaskDelay(1500); // wait 1.5 sec, so we see a different timestamp in the file...
	TelemetrySaveWOD();
	vTaskDelay(1500); // wait 1.5 sec, so we see a different timestamp in the file...
	TelemetrySaveWOD();
	vTaskDelay(1500); // wait 1.5 sec, so we see a different timestamp in the file...
	TelemetrySaveWOD();

	// read the data
	int numOfElementsSent = readTLMFile(tlm_wod,theDay,0,2,0);

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}




Boolean DeleteOldFiles(){

	// save current time
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	// set time to 2030/1/1
	Time_setUnixEpoch(1735689600);

	//delete the file
	Time theDay;
	theDay.year = 25;
	theDay.date = 1;
	theDay.month = 1;

	// delete all WOD files
	for (int i=0;i<5;i++){
		int err = deleteTLMFile(tlm_wod,theDay,i);
		err *= deleteTLMFile(tlm_antenna,theDay,i);
		err *= deleteTLMFile(tlm_eps,theDay,i);
		err *= deleteTLMFile(tlm_eps_eng_cdb,theDay,i);
		err *= deleteTLMFile(tlm_eps_eng_mb,theDay,i);
		err *= deleteTLMFile(tlm_eps_raw_cdb,theDay,i);
		err *= deleteTLMFile(tlm_log,theDay,i);
		err *= deleteTLMFile(tlm_rx,theDay,i);
		err *= deleteTLMFile(tlm_rx_frame,theDay,i);
		err *= deleteTLMFile(tlm_solar,theDay,i);
		err *= deleteTLMFile(tlm_tx,theDay,i);
	}

	// write some WOD elements in TLM file
	for (int i=0;i<5;i++){
		// set time to 2030/1/1
		Time_setUnixEpoch(1735689600 + (60*60*24*i));

		TelemetrySaveWOD();
		TelemetrySaveEPS();
		TelemetrySaveTRXVU();
		TelemetrySaveANT();
		TelemetrySaveSolarPanels();
	}

	DeleteOldFiels(MIN_FREE_SPACE*1000);

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}



Boolean CreateFiles4DeleteTest(){

	// save current time
	time_unix current_time = 0;
	Time_getUnixEpoch(&current_time);

	// set time to 2025/1/1
	Time_setUnixEpoch(1735689600);

	//delete the file
	Time theDay;
	theDay.year = 25;
	theDay.date = 1;
	theDay.month = 1;

	// delete all WOD files
	for (int i=0;i<10;i++){
		int err = deleteTLMFile(tlm_wod,theDay,i);

	}

	// write some WOD elements in TLM file
	for (int i=0;i<10;i++){
		// set time to 2030/1/1
		Time_setUnixEpoch(1735689600 + (60*60*24*i));
		TelemetrySaveWOD();
		vTaskDelay(1000);
		TelemetrySaveWOD();
		vTaskDelay(1000);
		TelemetrySaveWOD();

	}

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
	int numOfElementsSent = readTLMFile(tlm_wod,theDay,0,2,0);

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
		printf("*******  FAIL ***********\n");
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
		printf("*******  FAIL ***********\n");
	}else{
		printf("*******  PASS ***********\n");
	}

	// set the time back
	Time_setUnixEpoch(current_time);

	return TRUE;


}
Boolean TestReadMultiFiels(){

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

	strcpy(logData.msg, "test122");
	logData.error = 11;
	write2File(&logData,tlm_log);

	// set time to 2030/1/2
	Time_setUnixEpoch(1893542400);
	strcpy(logData.msg, "test222");
	logData.error = 20;
	write2File(&logData,tlm_log);

	strcpy(logData.msg, "test233");
	logData.error = 21;
	write2File(&logData,tlm_log);

	// set time to 2030/1/3
	Time_setUnixEpoch(1893628800);
	strcpy(logData.msg, "test333");
	logData.error = 30;
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
		printf("*******  FAIL ***********\n");
	}
	theDay.date = 1;
	int numOfElementsSent = readTLMFiles(tlm_log,theDay,2,2,0);


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
	int numOfElementsSent = readTLMFile(tlm_tx,theDay,0,2,0);
	printf("reading RX TLM\n");
	numOfElementsSent = readTLMFile(tlm_rx,theDay,0,2,0);

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
	printf("\t 3) LOG TLM (write & read 1 file with 2 lines)\n\r");
	printf("\t 4) WOD TLM \n\r");
	printf("\t 5) TRXVU TLM \n\r");
	printf("\t 6) Read multi files \n\r");
	printf("\t 7) Read with time range \n\r");
	printf("\t 8) Resolution (2 tests: full day & time range) \n\r");
	printf("\t 9) EPS TLM \n\r");
	printf("\t 10) Generate WOD TLM files \n\r");
	printf("\t 11) Delete OLD files \n\r");
	printf("\t 12) create files for delete test \n\r");
	//palmon is not a gever??

	unsigned int number_of_tests = 12;
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
	case 6:
		offerMoreTests = TestReadMultiFiels();
		break;
	case 7:
		offerMoreTests = TestReadTimeRangeTLM();
		break;
	case 8:
		offerMoreTests = TestReadTimeRangeTLMRes();
		break;
	case 9:
			offerMoreTests = TestEPSTLM();
			break;
	case 10:
			offerMoreTests = GenerateWODTLM();
			break;
	case 11:
			offerMoreTests = DeleteOldFiles();
			break;
	case 12:
			offerMoreTests = CreateFiles4DeleteTest();
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
