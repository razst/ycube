
#ifndef FS_COMMANDS_H_
#define FS_COMMANDS_H_
#include "SubSystemModules/Communication/SatCommandHandler.h"

/**
 * delete files from SD by passinf tlmtype, frotime, totime
 */
int CMD_DeleteTLM(sat_packet_t *cmd);

int CMD_DeleteFileByTime(sat_packet_t *cmd);

int CMD_DeleteFilesOfType(sat_packet_t *cmd);

int CMD_DeleteAllFiles(sat_packet_t *cmd);

int CMD_GetNumOfFilesInTimeRange(sat_packet_t *cmd);

int CMD_GetNumOfFilesByType(sat_packet_t *cmd);

int CMD_GetLastFS_Error(sat_packet_t *cmd);

int CMD_FreeSpace(sat_packet_t *cmd);

int CMD_GetFileLengthByTime(sat_packet_t *cmd);

int CMD_GetTimeOfLastElementInFile(sat_packet_t *cmd);

int CMD_GetTimeOfFirstElement(sat_packet_t *cmd);

int CMD_Get_TLM_Info(sat_packet_t *cmd);

int CMD_Switch_SD_Card(sat_packet_t *cmd);

int CMD_Format_SD_Card(sat_packet_t *cmd);

#endif /* FS_COMMANDS_H_ */
