#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"

int SendAckPacket(ack_subtype_t acksubtype, sat_packet_t *cmd,
		unsigned char *data, unsigned short length)
{
	int err = 0;
	sat_packet_t ack = { 0 };
	unsigned int id = 0;

	if (cmd != NULL) {
		id = cmd->ID;
	}else{
		id = 0x02FFFFFF; //default ID. system ACK. not a response to any command
	}

	AssembleCommand(data, length, (char)ack_type, (char)acksubtype, id, &ack);

	err = TransmitSplPacket(&ack,NULL);
	vTaskDelay(10);
	return err;
}

