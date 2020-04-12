/*
 * utils.c
 *
 *  Created on: Sep 19, 2019
 *      Author: pc
 */

#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <hal/errors.h>
#include "TLM_management.h"
#include <SubSystemModules/Housekepping/TelemetryFiles.h>


void timeU2time(time_unix utime, Time *time){
    struct tm  ts;
    char       buf[80];

	ts = *localtime(&utime);
	time->seconds = ts.tm_sec;
	time->minutes = ts.tm_min;
	time->hours = ts.tm_hour;
	time->date = ts.tm_mday;
	time->month = ts.tm_mon+1;
	time->year = ts.tm_year-100;

    //strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S", &ts);
    //printf("Local Time %s\n", buf);
	//printf("year: %d\n", time->year);

}

int logError(int error){
	if(error != E_NO_SS_ERR)
	{
		printf("%d", error); // TODO: remove before prod...

		logData_t log_;
		memcpy(log_.msg, "there was a error", MAX_LOG_STR);
		log_.error = error;
		write2File(&log_ , tlm_log);
		return 1;
	}
	return 0;

}

int logInfo(char *info){
	printf("info: %s\n",info);
	logData_t log_;
	memcpy(log_.msg, info, MAX_LOG_STR);
	log_.error = 0;
	write2File(&log_ , tlm_log);

	return 0;
}
