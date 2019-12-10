/*
 * utils.c
 *
 *  Created on: Sep 19, 2019
 *      Author: pc
 */

#include "utils.h"
#include <hal/errors.h>


int logError(int error){

//TODO LOG TO FILE
	if(error != E_NO_SS_ERR) return 1;
	return 0;

}

int logInfo(char *info , int size){
return 0;
}
