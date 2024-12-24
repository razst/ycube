#include "names.h"

#include "GlobalStandards.h"

void GetRandomName(char buffer[MAX_NAME_SIZE])
{
	time_unix time;
	Time_getUnixEpoch(&time);
	int index = time % sizeof(names);

	memcpy(buffer, names[index], sizeof(buffer));
}
