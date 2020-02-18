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


int logError(int error){
	if(error != E_NO_SS_ERR)
	{
		printf("%d", error); // TODO: remove before prod...

		logData log_;
		memcpy(log_.data, "there was a error", MAX_LOG_STR);
		log_.error = error;
		write2File(&log_ , tlm_log);
		return 1;
	}
	return 0;

}

int logInfo(char *info){
	logData log_;
	memcpy(log_.data, info, MAX_LOG_STR);
	log_.error = 0;
	write2File(&log_ , tlm_log);

	return 0;
}
