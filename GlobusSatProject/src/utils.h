/*
 * utils.h
 *
 *  Created on: Sep 19, 2019
 *      Author: pc
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "GlobalStandards.h"
#include <hal/Timing/Time.h>


#define TRXVU_TRANSPONDER_TOO_LONG		-199
#define E_CANT_TRANSMIT					-200
#define E_TOO_EARLY_4_BEACON			-201
#define TRXVU_MUTE_TOO_LONG				-202
#define TRXVU_IDLE_TOO_LONG				-203
#define E_INVALID_PARAMETERS			-204
#define TRXVU_IDLE_WHILE_TRANSPONDER	-205
#define BEACON_INTRAVL_TOO_SMALL		-206
#define SPL_DATA_TOO_BIG				-207
#define INVALID_TLM_TYPE				-208
#define INVALID_IMG_TYPE				-209
#define INFO_MSG						-210
#define TRXVU_TRANSPONDER_WHILE_MUTE	-211
#define TRXVU_IDEL_WHILE_MUTE			-212
#define PAYLOAD_TIMEOUT					-213
#define PAYLOAD_FALSE_OPERATION			-214
#define PAYLOAD_IS_DEAD					-215
#define E_UNAUTHORIZED					-216 ///< Passcode or id incorrect




#define MAX_ERRORS       				 4  // max errors we want to log from the same type toghether
#define MAX_TIME_BETWEEN_ERRORS          60 // max seconds we allow between errors
#define MAX_LOG_STR				100

typedef struct data
{
	int error;
	char msg[MAX_LOG_STR];
} logData_t;


/*
 * convert unix time to Time struct
 */
void timeU2time(time_unix utime, Time *time);
/*
 * log error message
 */
int logError(int error ,char* msg);



#endif /* UTILS_H_ */

