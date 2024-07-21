
#ifndef FILESYSTEMTESTINGDEMO_H_
#define FILESYSTEMTESTINGDEMO_H_

#include "GlobalStandards.h"
// TODO: move this struct to the struct place (and rename it)
typedef struct {
	unsigned short minDate;
	unsigned short maxDate;
	signed short lastError;
} mashooStruct;

Boolean MainFileSystemTestBench();

#endif /* FILESYSTEMTESTINGDEMO_H_ */
