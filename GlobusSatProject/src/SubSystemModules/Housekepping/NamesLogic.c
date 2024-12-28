#include "SubSystemModules/Housekepping/NamesLogic.h"
#include "GlobalStandards.h"


enum hebrewLetter // The second byte of each hebrew character as encoded with unicode. Add 0x05 before in order to use it
{
	AL = 0xD0,
	BE = 0xD1,
	GI = 0xD2,
	DA = 0xD3,
	HE = 0xD4,
	VA = 0xD5,
	ZA = 0xD6,
	CH = 0xD7,
	TE = 0xD8,
	YO = 0xD9,
	FKA = 0xDA,
	KA = 0xDB,
	LA = 0xDC,
	FME = 0xDD,
	ME = 0xDE,
	FNU = 0xDF,
	NU = 0xE0,
	SA = 0xE1,
	AI = 0xE2,
	FPE = 0xE3,
	PE = 0xE4,
	FTS = 0xE5,
	TS = 0xE6,
	KU = 0xE7,
	RE = 0xE8,
	SH = 0xE9,
	TA = 0xEA
};

char beacon_names[NAMES_COUNT][MAX_NAME_SIZE] = {
	{ DA, RE, VA, RE },
	{ AL, VA, RE, YO, AL, LA },
	{ NU, VA, HE },
	{ RE, ZA },
	{ YO, SH, YO }
};




void GetRandomName(char *buffer)
{
	time_unix time;
	Time_getUnixEpoch(&time);
	int index = time % NAMES_COUNT;

	memcpy(buffer, beacon_names[index], MAX_NAME_SIZE);
}
