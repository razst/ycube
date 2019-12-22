#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"

int SendAckPacket(ack_subtype_t acksubtype, sat_packet_t *cmd,
		unsigned char *data, unsigned int length)
{
	int err = 0;
	sat_packet_t ack = { 0 };
	unsigned int id = 0;

	if (cmd != NULL) {
		id = cmd->ID;
	}else{
		id = 0xFFFFFFFF; //default ID. system ACK. not a response to any command
	}

	AssembleCommand(data, length, (char)ack_type, (char)acksubtype, id, &ack);

	err = TransmitSplPacket(&ack,NULL);
	vTaskDelay(10); // TODO why do we need to wait and why 10 ticks ???
	return err;
}

void SendErrorMSG(ack_subtype_t fail_subt, ack_subtype_t succ_subt,
		sat_packet_t *cmd, int err)
{
}

void SendErrorMSG_IfError(ack_subtype_t subtype, sat_packet_t *cmd, int err)
{
}

