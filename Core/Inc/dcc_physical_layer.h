/*
 * dcc_protocol.h
 *
 *  Created on: Mar 21, 2024
 *      Author: J. Früh
 */

#ifndef INC_DCC_PHYSICAL_LAYER_H_
#define INC_DCC_PHYSICAL_LAYER_H_

#include "main.h"

#define DCC_MAX_MESSAGE_LEN 10
#define DCC_MAX_MESSAGES_QUEUE 16	// Has to be a power of 2

// How many time a message has to be sent
#define DCC_MSG_REPEATED_TIME 100


#define DCC_RX_PREAMBLE_INIT 21
#define DCC_RX_ZERO_LOW 17
#define DCC_RX_ZERO_HIGH 22
#define DCC_RX_ONE_LOW 10
#define DCC_RX_ONE_HIGH 14


typedef struct{
	uint8_t len;
	uint8_t data[DCC_MAX_MESSAGE_LEN];
}MESSAGE_STRUCT;

typedef struct{
	uint8_t preamble_i;
	uint8_t bit_i;
	uint8_t byte_i;
	uint8_t msg_in_i;
	uint8_t msg_out_i;
	uint8_t sent_nbr;
	MESSAGE_STRUCT msg[DCC_MAX_MESSAGES_QUEUE];
} DCC_PROTOCOL_STRUCT;

typedef struct{
	uint32_t val_buffer[30];
	uint8_t buffer[64];
	uint32_t idx;
	uint32_t recieved_msg;
	uint32_t first_start_error;
	uint32_t bit_size_error;
	uint32_t msg_len_error;
	uint32_t xor_error;
} DCC_DEBUG_STRUCT;


void dcc_init(void);

void dcc_tx_update(void);

void dcc_rx_update(void);

#endif /* INC_DCC_PHYSICAL_LAYER_H_ */
