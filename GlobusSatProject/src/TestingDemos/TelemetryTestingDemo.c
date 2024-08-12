#include "TelemetryTestingDemo.h"

Boolean singleTlmTest()
{
	int amount;
	logData_t data1;
	data1.error = 1;
	char str[6] = "hello";
	memcpy(&data1.msg, str, sizeof(str));
	printf("saving single log:\n");
	printf("error: %d\n", data1.error);
	printf("message: %s\n", data1.msg);

	saveTlmToRam(&data1, sizeof(data1), tlm_log);

	logDataInRam log;
	amount = getTlm(&log, 1, tlm_log);
	printf("Got %d datas\n", amount);
	printf("error: %d\nmessage: %s\n", log.logData.error, log.logData.msg);

	return data1.error == log.logData.error;
}

Boolean multipleTlmTest()
{
	logData_t arr[10];
	logData_t arr2[10];
	Boolean flg = TRUE;

	for(int i = 0; i < 10; i++)
	{
		arr[i].error = i+1;
		sprintf(arr[i].msg ,"hello%d", i+1);

		printf("saving log %d:\n", i+1);
		printf("error: %d\n", arr[i].error);
		printf("message: %s\n", arr[i].msg);

		saveTlmToRam(&arr[i], sizeof(arr[i]), tlm_log);
	}

	getTlm(arr2, 10, tlm_log);
	for(int i = 0; i < 10; i++)
	{
		printf("log %d:\n", i+1);
		printf("error: %d\n", arr2[i].error);
		printf("message: %s\n\n", arr2[i].msg);

		flg = flg && (arr[i].error == arr2[i].error);
	}


	return flg;
}

Boolean tlmTest()
{
	printf("\nSingle data test:\n\n");

	if(singleTlmTest())
	{
		printf("single Test passed!\n");
	}
	else
	{
		printf("single Test failed!\n");
	}

	printf("\nMultiple data test:\n\n");
	if(multipleTlmTest())
	{
		printf("multiple Test passed!\n");
	}
	else
	{
		printf("multiple Test failed!\n");
	}


	printf("\nReset test:\n\n");
	resetArrs();

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

		unsigned int number_of_tests = 1;
		while(UTIL_DbguGetIntegerMinMax(&choice, 0, number_of_tests) == 0);

			switch(choice) {
			case 0:
				offerMoreTests = FALSE;
				break;
			case 1:
				offerMoreTests = tlmTest();
				break;
			}
	}
	return FALSE;
}
