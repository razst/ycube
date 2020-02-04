#include <satellite-subsystems/IsisTRXVU.h>
#include <hal/Timing/Time.h>
#include <string.h>
#include <stdlib.h>

#include "GlobalStandards.h"
#include "SatCommandHandler.h"
#include "SPL.h"


typedef struct __attribute__ ((__packed__)) delayed_cmd_t
{
	time_unix exec_time;	///< the execution time of the cmd in unix time
	sat_packet_t cmd;		///< command data
} delayed_cmd_t;

int ClearDelayedCMD_FromBuffer(unsigned int start_addr, unsigned int end_addr)
{
	return 0;
}

int ParseDataToCommand(unsigned char * data, sat_packet_t *cmd)
{
	if(NULL == data || NULL == cmd){
		return null_pointer_error;
	}
	void *err = NULL;

	unsigned int offset = 0;

	unsigned int id = 0;
	err = memcpy(&id,data,sizeof(id));
	if (NULL == err) {
		return execution_error;
	}
	offset += sizeof(id);

	char type;
	err = memcpy(&type,data+offset,sizeof(type));
	if (NULL == err) {
		return execution_error;
	}
	offset += sizeof(type);

	char subtype;
	err = memcpy(&subtype, data + offset,sizeof(subtype));
	if (NULL == err) {
		return execution_error;
	}
	offset += sizeof(subtype);

	unsigned int data_length = 0; // TODO in the SPL presentation, it says that length is 16bits and note 32 bits, who is correct??
	err = memcpy(&data_length, data + offset,sizeof(data_length));
		if (NULL == err) {
			return execution_error;
		}
	offset += sizeof(data_length);

	return AssembleCommand(data+offset,data_length,type,subtype,id,cmd);

}

int AssembleCommand(unsigned char *data, unsigned int data_length, char type,
		char subtype, unsigned int id, sat_packet_t *cmd)
{
	if (NULL == cmd) {
		return null_pointer_error;
	}
	cmd->ID = id;
	cmd->cmd_type = type;
	cmd->cmd_subtype = subtype;
	cmd->length = 0;

	if (NULL != data) {
		unsigned int size =(data_length > MAX_COMMAND_DATA_LENGTH) ?
				MAX_COMMAND_DATA_LENGTH :data_length; // TODO, if data_length > MAX_COMMAND_DATA_LENGTH than log an error
		cmd->length = size;
		void *err = memcpy(cmd->data, data, size);

		if (NULL == err) {
			return execution_error;
		}
	}
	return command_succsess;
}

// checks if a cmd time is valid for execution -> execution time has passed and command not expired
// @param[in] cmd_time command execution time to check
// @param[out] expired if command is expired the flag will be raised
Boolean isDelayedCommandDue(time_unix cmd_time, Boolean *expired)
{
	return FALSE;
}

//TOOD: move delayed cmd logic to the SD and write 'checked/uncheked' bits in the FRAM
int GetDelayedCommand(sat_packet_t *cmd)
{
	return 0;
}

int AddDelayedCommand(sat_packet_t *cmd)
{
	return 0;
}

int GetDelayedCommandBufferCount()
{
	unsigned char frame_count = 0;
	int err = FRAM_read(&frame_count, DELAYED_CMD_FRAME_COUNT_ADDR,
	DELAYED_CMD_FRAME_COUNT_SIZE);
	return err ? -1 : frame_count;
}


int GetDelayedCommandByIndex(unsigned int index, sat_packet_t *cmd)
{
	return 0;
}

int DeleteDelayedCommandByIndex(unsigned int index)
{
	return 0;
}

int DeleteDelayedBuffer()
{
	return 0;
}


int ActUponCommand(sat_packet_t *cmd)
{
	int err = 0;
	if (NULL == cmd){
		return E_NOT_INITIALIZED;
	}

	switch (cmd->cmd_type)
	{
	case trxvu_cmd_type:
		err = trxvu_command_router(cmd);
		break;
	case eps_cmd_type:
		err = eps_command_router(cmd);
		break;
	case telemetry_cmd_type:
		err = telemetry_command_router(cmd);
		break;
	case filesystem_cmd_type:
		err = filesystem_command_router(cmd);
		break;
	case managment_cmd_type:
		err = managment_command_router(cmd);
		break;
	case ack_type: 								//this command is a ping function
		SendAckPacket(ACK_PING, cmd,NULL,0);
		break;
	}
	return err;
}
