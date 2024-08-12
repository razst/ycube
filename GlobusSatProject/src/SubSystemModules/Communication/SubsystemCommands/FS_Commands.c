#include "FS_Commands.h"
#include "GlobalStandards.h"
#include "TLM_management.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include <hcc/api_fat.h>
#include "TLM_management.h"
#include <hcc/api_mdriver_atmel_mcipdc.h>



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

int CMD_DeleteAllFiles(sat_packet_t *cmd)
{
	delete_allTMFilesFromSD();
	SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
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
	int err = f_getlasterror();
	if (err == E_NO_SS_ERR)
	{
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)&err, sizeof(int));
	}
	return err;

}
int CMD_Get_TLM_Info(sat_packet_t *cmd)
{
	TLM_Info_Data_t data;
	FN_SPACE space = { 0 };
	int drivenum = f_getdrive();
	// get the free space of the SD card
	int err = logError(f_getfreespace(drivenum, &space), "CMD_Get_TLM_Info");

	unsigned short* minMaxDate = findMinMaxDate();
	if(err == E_NO_SS_ERR)
	{
		data.total = space.total;
		data.used = space.used;
		data.free = space.free;
		data.bad = space.bad;
		data.minFileDate = minMaxDate[0];
		data.maxFileDate = minMaxDate[1];
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)&data, sizeof(data));
	}
	return err;

}
int CMD_Switch_SD_Card(sat_packet_t *cmd)
{
	char change_to_SD_card,current_SD_card;
	memcpy(&change_to_SD_card,cmd->data,sizeof(change_to_SD_card));
	FRAM_read((unsigned char*)&current_SD_card,ACTIVE_SD_ADDR,ACTIVE_SD_SIZE);
	if(current_SD_card == change_to_SD_card)
	{
		return E_IS_INITIALIZED;
	}
	int err = FRAM_write((unsigned char*)&change_to_SD_card,ACTIVE_SD_ADDR, ACTIVE_SD_SIZE);
	//TODO check after 2nd SD is added
	if(E_NO_SS_ERR == err)
	{
		restart();
		vTaskDelay(10000);
	}
	return err;
}
int CMD_Format_SD_Card(sat_packet_t *cmd)
{
	int drivenum;
	drivenum = f_getdrive();
	return logError(f_format(drivenum, F_FAT32_MEDIA),"Format SD");
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
