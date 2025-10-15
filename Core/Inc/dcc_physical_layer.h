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
//#define DCC_MSG_REPEATED_TIME 100

#define DMA_TIME_BUFFER_LENGTH 64
#define DMA_TIME_IDX_MASK (DMA_TIME_BUFFER_LENGTH-1)


#define DCC_RX_PREAMBLE_INIT 21
#define DCC_RX_ZERO_LOW 17
#define DCC_RX_ZERO_HIGH 22
#define DCC_RX_ONE_LOW 10
#define DCC_RX_ONE_HIGH 14


typedef struct{
	//uint8_t gpio_buffer[DMA_DCC_BUFFER_LENGTH];
	uint32_t val[3];
	uint32_t val_idx;
	uint32_t t_low;
	uint32_t t_high;

	uint32_t idx_in;
	uint32_t idx_out0;
	uint32_t idx_out1;
	uint16_t time_buffer[DMA_TIME_BUFFER_LENGTH];
} DCC_PHYSICAL_LAYER_STRUCT;


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
	uint16_t timeout;
	MESSAGE_STRUCT msg[DCC_MAX_MESSAGES_QUEUE];
} DCC_PROTOCOL_STRUCT;

typedef struct{
	uint16_t green_cnt;
	uint16_t yellow_cnt;
	uint16_t red_cnt;
	//signal_state_t signal_state;
	uint16_t timeout_tab[8];
	uint32_t in_idx;
	uint32_t out_idx;
	//uint16_t val[300];
	//uint32_t val_index;
} DCC_SIGNAL_STRUCT;

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

typedef struct{
	uint8_t gpio[2048];
	uint32_t idx;
} DCC_DEBUG2_STRUCT;

void dcc_physical_layer_init(void);

void dcc_tx_update(void);

void dcc_dma_update(uint32_t buffer_full);

void dcc_rx_update(void);

void signal_update(void);

#endif /* INC_DCC_PHYSICAL_LAYER_H_ */
