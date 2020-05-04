
#ifndef TRXVU_COMMANDS_H_
#define TRXVU_COMMANDS_H_

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"

// wait time for taking semaphores
#define WAIT_TIME_SEM_DUMP	1 //TODO: verify how much time we need to wait
#define WAIT_TIME_SEM_TX	1 //TODO: verify how much time we need to wait

#define ANTENNA_DEPLOYMENT_TIMEOUT 10 //<! in seconds


int CMD_StartDump(sat_packet_t *cmd);

int CMD_SendDumpAbortRequest(sat_packet_t *cmd);

int CMD_ForceDumpAbort(sat_packet_t *cmd);

int CMD_MuteTRXVU(sat_packet_t *cmd);
/**
 * set trxvu idle state
 * cmd data should include 0x01 (On) or 0x00 (Off). if sent ON than also pass the duration (4 bytes int)
 */
int CMD_SetIdleState(sat_packet_t *cmd);

int CMD_UnMuteTRXVU(sat_packet_t *cmd);

int CMD_GetBaudRate(sat_packet_t *cmd);

int CMD_SetBaudRate(sat_packet_t *cmd);

int CMD_GetBeaconInterval(sat_packet_t *cmd);


int CMD_GetTxUptime(sat_packet_t *cmd);

int CMD_GetRxUptime(sat_packet_t *cmd);

int CMD_GetNumOfDelayedCommands(sat_packet_t *cmd);

int CMD_GetNumOfOnlineCommands(sat_packet_t *cmd);

int CMD_DeleteDelyedCmdByID(sat_packet_t *cmd);

int CMD_DeleteAllDelyedBuffer(sat_packet_t *cmd);

int CMD_AntSetArmStatus(sat_packet_t *cmd);

int CMD_AntGetArmStatus(sat_packet_t *cmd);

int CMD_AntGetUptime(sat_packet_t *cmd);

int CMD_AntCancelDeployment(sat_packet_t *cmd);

int CMD_AntennaDeploy(sat_packet_t *cmd);


int CMD_StopReDeployment(sat_packet_t *cmd);

#endif
/*COMMANDS_H_ */
