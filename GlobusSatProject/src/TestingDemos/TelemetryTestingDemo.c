#include "TelemetryTestingDemo.h"

//get number of packets and save it in RAM with modified data
void savePacketsInRam(int amount, tlm_type_t type)
{
	switch (type)
	{
	case tlm_log:
		for(int i = 1; i <= amount; i++)
		{

			logData_t data;
			data.error = i;
			sprintf(data.msg ,"hello%d", i);

			printf("saving log %d:\nError code: %d\nMessage: %s\n", i, data.error, data.msg);
			saveTlmToRam(&data, sizeof(data), type);
		}
		break;

	case tlm_wod:
		for(int i = 1; i <= amount; i++)
		{
			WOD_Telemetry_t data;
			data.free_memory = i;

			saveTlmToRam(&data, sizeof(data), type);
		}
		break;
	}

}

//get a packet and check if it is as we expected according to the parameter
Boolean checkPacket(logData_t* data, int number)
{
	Boolean flag = TRUE;
	char str[10];
	if (data->error != number)
	{
		printf("Error: error code wasn't as expected\n");
		flag = FALSE;
	}
	sprintf(str, "hello%d", number);
	if (strcmp(data->msg, str) != 0)
	{
		printf("Error: error msg wasn't as expected\n");
		flag = FALSE;
	}

	return flag;
}

//test to save 1 WOD packet and get it
Boolean singleTlmWodTest()
{
	int amount;
	Boolean flag = TRUE;
	wodDataInRam data_recieved;

	savePacketsInRam(1, tlm_wod);

	amount = getTlm(&data_recieved, 1, tlm_wod);

	printf("Got %d datas\nFree memory: %d\n\n", amount, data_recieved.wodData.free_memory);

	if (amount != 1)
	{
		printf("Error: amount should be 1\n");
		flag = FALSE;
	}
	if (data_recieved.wodData.free_memory != 1)
	{
		printf("Error: free memory wasn't as expected\n");
		flag = FALSE;
	}

	return flag;
}




//test to save 1 LOG packet and get it
Boolean singleTLMLogTest()
{
	int amount;
	logDataInRam data_recieved;
	Boolean flag = TRUE;

	savePacketsInRam(1, tlm_log);

	amount = getTlm(&data_recieved, 1, tlm_log);

	printf("Got %d datas\nError: %d\nMessage: %s\n\n", amount, data_recieved.logData.error, data_recieved.logData.msg);

	if (amount != 1)
	{
		printf("Error: amount should be 1\n");
		flag = FALSE;
	}

	return checkPacket(&data_recieved.logData, 1) && flag;
}

Boolean singleTlmLogTestHelp()
{
	Boolean flag = singleTLMLogTest();
	if(flag)
	{
		printf("test successful");
	}
	else
	{
		printf("test failed");
	}
	return TRUE;
}

//test to save multiple LOG packets according to the parameter
Boolean saveNGetMultipleTlmTest(int amount)
{
	if(amount > TLM_RAM_SIZE * 4)
	{
		return FALSE;
	}

	int count;

	logDataInRam arr[TLM_RAM_SIZE * 4]; //for all cases

	printf("try saving %d packets\n", amount);
	Boolean flag = TRUE;

	savePacketsInRam(amount, tlm_log);

	count = getTlm(arr, amount, tlm_log);
	printf("Get %d datas\n", count);

	//check counts of data recieved:
	if (amount <= TLM_RAM_SIZE && count != amount)
	{
		printf("Error: amount should be %d\n", amount);
		flag = FALSE;
	}
	else if(amount > TLM_RAM_SIZE && count != TLM_RAM_SIZE)
	{
		printf("Error: amount should be %d\n", TLM_RAM_SIZE);
		flag = FALSE;
	}

	//check content of packets:
	for(int i = 0; i < amount; i++)
	{
		printf("packet %d\nError code: %d\nMessage: %s\n\n", i+1, arr[i].logData.error, arr[i].logData.msg);

		flag = flag && checkPacket(&arr[i].logData, i+1);
	}

	return flag;
}

Boolean getWodMinMaxTest()
{
	resetRamTlm();
	dataRange range = getRange(tlm_wod);
	if(range.max != 0 || range.min != 0)
	{
		printf("reset failed\n");
	}
	else
	{
		printf("reset passed\n");
	}

	time_unix min_date;
	Time_getUnixEpoch(&min_date);
	savePacketsInRam(3, tlm_wod);

	vTaskDelay(2000);

	time_unix max_date;
	Time_getUnixEpoch(&max_date);
	savePacketsInRam(3, tlm_wod);

	time_unix maxRange = getRange(tlm_wod).max;
	time_unix minRange = getRange(tlm_wod).min;
	if(abs(minRange-min_date) > 1 || abs(maxRange-max_date) > 1)
	{
		printf("min max test failed\n");
		printf("Range min: %lu  Min date: %lu\nRange max: %lu  max date: %lu\n",
				minRange, min_date, maxRange, max_date);
	}
	else
	{
		printf("min max test passed\n");
	}

	return TRUE;
}

//main function to call all the tests
Boolean tlmTest()
{
	printf("WOD single packet test:\n\n");
	if(singleTlmWodTest())
	{
		printf("single WOD Test passed!\n");
	}
	else
	{
		printf("single WOD Test failed!\n");
	}

	printf("\nLOG TESTS:\n\nSingle data test:\n\n");

	if(singleTLMLogTest())
	{
		printf("single Test passed!\n");
	}
	else
	{
		printf("single Test failed!\n");
	}

	printf("\nTLM_RAM_SIZE: %d\nTest for half:\n\n", TLM_RAM_SIZE);

	if(saveNGetMultipleTlmTest(TLM_RAM_SIZE/2))
	{
		printf("Half ram size Test passed!\n");
	}
	else
	{
		printf("Half ram size Test failed!\n");
	}

	printf("\nTLM_RAM_SIZE: %d\nTest for ram size:\n\n", TLM_RAM_SIZE);

	if(saveNGetMultipleTlmTest(TLM_RAM_SIZE))
	{
		printf("Ram size Test passed!\n");
	}
	else
	{
		printf("Ram size Test failed!\n");
	}

	printf("\nTLM_RAM_SIZE: %d\nTest for 3 times ram size:\n\n", TLM_RAM_SIZE);

	if(saveNGetMultipleTlmTest(TLM_RAM_SIZE*3))
	{
		printf("3 * ram size Test passed!\n");
	}
	else
	{
		printf("3 * ram size Test failed!\n");
	}


	printf("\nReset test:\n\n");
	resetRamTlm();

	logDataInRam log;
	int amount = getTlm(&log, 1, tlm_log);
	if(amount == 0)
	{
		printf("Reset test passed!\n");
	}
	else
	{
		printf("Reset test failed!\n");
	}

	//TODO - add test for finding range of TLM
}

Boolean MainTelemetryTestBench()
{
	Boolean offerMoreTests = TRUE;
	unsigned int choice;

	while(offerMoreTests)
	{
		printf( "\n\r Select a test to perform: \n\r");
		printf("\t 0) Return to main menu \n\r");
		printf("\t 1) tlm in RAM full test \n\r");
		printf("\t 2) single tlm in ram test \n\r");
		printf("\t 3) min max dates wod in ram test \n\r");

		unsigned int number_of_tests = 3;
		while(UTIL_DbguGetIntegerMinMax(&choice, 0, number_of_tests) == 0);

			switch(choice) {
			case 0:
				offerMoreTests = FALSE;
				break;
			case 1:
				offerMoreTests = tlmTest();
				break;
			case 2:
				offerMoreTests = singleTlmLogTestHelp();
				break;
			case 3:
				offerMoreTests = getWodMinMaxTest();
				break;
			}
	}
	return FALSE;
}
