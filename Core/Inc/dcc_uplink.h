/*
 * dcc_reverse_com.h
 *
 *  Created on: Jul 9, 2026
 *      Author: J. Frueh
 */

#ifndef INC_DCC_UPLINK_H_
#define INC_DCC_UPLINK_H_

#include <stdint.h>


#define DCC_REVERSE_COM_BYTE_COUNT 4U
#define DCC_REVERSE_COM_START_BIT 0U
#define DCC_REVERSE_COM_STOP_BIT 1U
#define DCC_REVERSE_COM_MESSAGE_GAP_BITS 11U
#define DCC_REVERSE_COM_ADDRESS_MASK 0x3FFFU


void dcc_reverse_com_init(void);
void dcc_reverse_com_tick(void);
void dcc_reverse_com_start(uint16_t module_address);
void dcc_reverse_com_stop(void);

#endif /* INC_DCC_UPLINK_H_ */
