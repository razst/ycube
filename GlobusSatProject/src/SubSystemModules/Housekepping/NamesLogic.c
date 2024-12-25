#include "SubSystemModules/Housekepping/NamesLogic.h"
#include "GlobalStandards.h"


char beacon_names[NAMES_COUNT][MAX_NAME_SIZE] = {"ishay", "uriel", "nave", "raz", "dror"};


void GetRandomName(char *buffer)
{
	time_unix time;
	Time_getUnixEpoch(&time);
	int index = time % NAMES_COUNT;

	memcpy(buffer, beacon_names[index], sizeof(beacon_names[index]));
}
