#include "FS_Commands.h"
#include "GlobalStandards.h"
#include "TLM_management.h"
#include "SubSystemModules/Communication/TRXVU.h"



int CMD_DeleteTLM(sat_packet_t *cmd)
{
	if (NULL == cmd) {
		return -1;
	}

	dump_arguments_t dmp_pckt;
	unsigned int offset = 0;

	AssembleCommand(&cmd->data,cmd->length,cmd->cmd_type,cmd->cmd_subtype,cmd->ID, &dmp_pckt.cmd);

	memcpy(&dmp_pckt.dump_type, cmd->data, sizeof(dmp_pckt.dump_type));
	offset += sizeof(dmp_pckt.dump_type);

	memcpy(&dmp_pckt.t_start, cmd->data + offset, sizeof(dmp_pckt.t_start));
	offset += sizeof(dmp_pckt.t_start);

	memcpy(&dmp_pckt.t_end, cmd->data + offset, sizeof(dmp_pckt.t_end));
	offset += sizeof(dmp_pckt.t_end);


	// calculate how many days we were asked to dump (every day has 86400 seconds)
	int numberOfDays = (dmp_pckt.t_end - dmp_pckt.t_start)/86400;
	Time date;
	timeU2time(dmp_pckt.t_start,&date);
	int numOfElementsSent = deleteTLMFiles(dmp_pckt.dump_type,date,numberOfDays);
	SendAckPacket(ACK_DELETE_TLM, &cmd, numOfElementsSent, sizeof(numOfElementsSent));
	return 0;
}


int CMD_DeleteFileByTime(sat_packet_t *cmd)
{
	return 0;
}

int CMD_DeleteFilesOfType(sat_packet_t *cmd)
{
	return 0;
}

int CMD_DeleteFS(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetNumOfFilesInTimeRange(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetNumOfFilesByType(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetLastFS_Error(sat_packet_t *cmd)
{
	return 0;
}

int CMD_FreeSpace(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetFileLengthByTime(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetTimeOfLastElementInFile(sat_packet_t *cmd)
{
	return 0;
}

int CMD_GetTimeOfFirstElement(sat_packet_t *cmd)
{
	return 0;
}
