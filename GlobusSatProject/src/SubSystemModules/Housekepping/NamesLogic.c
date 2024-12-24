#include "SubSystemModules/Housekepping/NamesLogic.h"
#include "GlobalStandards.h"

void GetRandomName(char buffer[MAX_NAME_SIZE])
{
	time_unix time;
	Time_getUnixEpoch(&time);
	int index = time % NAMES_COUNT;

	memcpy(buffer, names[index], sizeof(buffer));
}
