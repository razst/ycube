/*
 * main.c
 *      Author: Akhil
 */
#include <satellite-subsystems/version/version.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Timing/Time.h>
#include <hal/Drivers/LED.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/boolean.h>
#include <hal/version/version.h>

#include <hal/Storage/FRAM.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sub-systemCode/COMM/GSC.h"
#include "sub-systemCode/Main/commands.h"
#include "sub-systemCode/Main/Main_Init.h"
#include "sub-systemCode/Global/Global.h"
#include "sub-systemCode/EPS.h"

#define FIRST_ACTIVATION_FLAG_ADDR		0x42		//<! is this the first activation after launch flag
#define FIRST_ACTIVATION_FLAG_SIZE		4			//<! length in bytes of FIRST_ACTIVATION_FLAG


#define HK_DELAY_SEC 10
#define MAX_SIZE_COMMAND_Q 20

void save_time()
{
	int err;
	// get current time from time module
	time_unix current_time;
	err = Time_getUnixEpoch(&current_time);
	check_int("set time, Time_getUnixEpoch", err);
	byte raw_time[TIME_SIZE];
	// converting the time_unix to raw (small endien)
	raw_time[0] = (byte)current_time;
	raw_time[1] = (byte)(current_time >> 8);
	raw_time[2] = (byte)(current_time >> 16);
	raw_time[3] = (byte)(current_time >> 24);
	// writing to FRAM the current time
	err = FRAM_write(raw_time, TIME_ADDR, TIME_SIZE);
	check_int("set time, FRAM_write", err);
}

void Command_logic()
{
	TC_spl command;
	int error = 0;
	do
	{
		error = get_command(&command);
		if (error == 0)
			act_upon_command(command);
	}
	while (error == 0);
}

void taskMain()
{
	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);
	InitSubsystems();

	vTaskDelay(100);
	printf("init\n");
	SubSystemTaskStart();
	printf("Task Main start\n");

	portTickType xLastWakeTime = xTaskGetTickCount();
	const portTickType xFrequency = 1000;

	while(1)
	{
		EPS_Conditioning();
		Command_logic();
		save_time();
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

Boolean getFirstActivation()
{
	Boolean flag = FALSE;
	FRAM_read((unsigned char*) &flag, FIRST_ACTIVATION_FLAG_ADDR,
	FIRST_ACTIVATION_FLAG_SIZE);
	return flag;
}


void setFirstActivation(Boolean value){
	FRAM_write((unsigned char*) &value,
	FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);

}

int main()
{
	TRACE_CONFIGURE_ISP(DBGU_STANDARD, 2000000, BOARD_MCK);
	// Enable the Instruction cache of the ARM9 core. Keep the MMU and Data Cache disabled.
	CP15_Enable_I_Cache();

	printf("Start FirstActivation program \n");

	Boolean firstActivationValue = getFirstActivation();
	printf("First activation value in FRAM:%d \n",firstActivationValue);

	setFirstActivation(TRUE);

	printf("First activation value in FRAM:%d \n",firstActivationValue);
	return 0;
}
