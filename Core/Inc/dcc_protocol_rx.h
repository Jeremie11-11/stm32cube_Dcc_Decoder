/*
 * dcc_decoder.h
 *
 *  Created on: 28 mars 2024
 *      Author: J. Früh
 */

#ifndef INC_DCC_PROTOCOL_RX_H_
#define INC_DCC_PROTOCOL_RX_H_

#include "main.h"


#define ADDR_BROADCAST 0x00
#define ADDR_UNVALID 0xFFFF



typedef enum{
	decoder_consist_ctrl_inst = 0,
	adv_operation_inst = 1,
	speed_dir_inst_reverse = 2,
	speed_dir_inst_forward = 3,
	function_group_one_inst = 4,
	function_group_two_inst = 5,
	feature_expansion = 6,
	config_variable_inst = 7,
}inst_type_enum;



typedef struct{
	uint8_t nb_data;
	uint16_t addr;
	uint8_t inst;
	uint8_t speed;
	uint8_t emergency;
	uint8_t fct;
	uint8_t mem_idx;
	uint8_t mem_data;
} DCC_MESSAGE_STRUCT;

typedef struct{
	signal_state_t signal_state;
	int8_t dcc_target_speed;	// Target speed received from DCC
	int8_t target_speed;			// Target speed based on DCC target speed and signal state
	int8_t actual_speed;			// Speed reference for motor control with ramp
	dir_t actual_dir;					// Used for the lights (following moving directions)
	uint8_t functions;				// Byte where the function are stored(F
	uint8_t emergency_stop;
} DCC_INSTRUCTION_STRUCT;


void dcc_decode(DCC_INSTRUCTION_STRUCT *dcc, uint8_t *buffer, uint8_t len);

void dcc_check_for_new_messages(void);

void dcc_update_functions(void);

#endif /* INC_DCC_PROTOCOL_RX_H_ */
