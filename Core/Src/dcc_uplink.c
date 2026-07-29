/*
 * dcc_reverse_com.c
 *
 *  Created on: Jul 9, 2026
 *      Author: J. Frueh
 */

#include <dcc_uplink.h>
#include "i_gpio.h"


// One message contains a synchronization byte, a 14-bit address and an XOR checksum.
static uint8_t reverse_com_data[DCC_REVERSE_COM_BYTE_COUNT] = {0x55, 0x3F, 0xFF, 0x95};
static uint8_t reverse_com_byte_idx = 0U;
static uint8_t reverse_com_bit_idx = 0U;
static uint8_t reverse_com_gap_bits = 0U;
static uint8_t reverse_com_active = FALSE;

static uint8_t dcc_reverse_com_get_bit(void);
static void dcc_reverse_com_write_bit(uint8_t bit);

/*
 * Set the output to its idle state.
 * Called once by tim_init(); not timing-critical.
 */
void dcc_reverse_com_init(void)
{
	dcc_reverse_com_write_bit(DCC_REVERSE_COM_STOP_BIT);
}

/*
 * Send one bit or one idle period.
 * Called by the TIM16 interrupt; timing-critical and must remain short.
 */
void dcc_reverse_com_tick(void)
{
	if(reverse_com_active == FALSE)
	{
		dcc_reverse_com_write_bit(DCC_REVERSE_COM_STOP_BIT);
		return;
	}

	// Keep the line idle for 10 Tbit between two messages.
	if(reverse_com_gap_bits > 0U)
	{
		dcc_reverse_com_write_bit(DCC_REVERSE_COM_STOP_BIT);
		reverse_com_gap_bits--;
		return;
	}

	dcc_reverse_com_write_bit(dcc_reverse_com_get_bit());

	reverse_com_bit_idx++;
	if(reverse_com_bit_idx >= 10U)
	{
		reverse_com_bit_idx = 0U;
		reverse_com_byte_idx++;

		if(reverse_com_byte_idx >= DCC_REVERSE_COM_BYTE_COUNT)
		{
			reverse_com_byte_idx = 0U;
			reverse_com_gap_bits = DCC_REVERSE_COM_MESSAGE_GAP_BITS;
		}
	}
}

/*
 * Load and restart a reverse communication message.
 * Public API with no current caller; not timing-critical.
 */
void dcc_reverse_com_start(uint16_t module_address)
{
	if(reverse_com_active == FALSE)
	{
		module_address &= DCC_REVERSE_COM_ADDRESS_MASK;

		reverse_com_data[0] = 0x55;
		reverse_com_data[1] = (uint8_t)(module_address >> 8);
		reverse_com_data[2] = (uint8_t)module_address;
		reverse_com_data[3] = reverse_com_data[0] ^ reverse_com_data[1] ^ reverse_com_data[2];

		reverse_com_byte_idx = 0U;
		reverse_com_bit_idx = 0U;
		reverse_com_gap_bits = 0U;
		reverse_com_active = TRUE;
	}
}

void dcc_reverse_com_stop()
{
	reverse_com_active = FALSE;
}

/*
 * Return the bit selected by the current transmission indexes.
 * Called only by dcc_reverse_com_tick(); timing-critical.
 */
static uint8_t dcc_reverse_com_get_bit(void)
{
	uint8_t byte = reverse_com_data[reverse_com_byte_idx];

	// Each byte is sent as one start bit, eight data bits LSB first and one stop bit.
	if(reverse_com_bit_idx == 0U)
	{
		return DCC_REVERSE_COM_START_BIT;
	}

	if(reverse_com_bit_idx >= 9U)
	{
		return DCC_REVERSE_COM_STOP_BIT;
	}

	return (byte >> (reverse_com_bit_idx - 1U)) & 0x01U;
}

/*
 * Apply one logical bit to the inverted physical output.
 * Called during initialization and from the TIM16 interrupt; timing-critical in the interrupt.
 */
static void dcc_reverse_com_write_bit(uint8_t bit)
{
	// The reverse communication output is electrically inverted.
	GPIO_WRITE(TEST_PIN_REVERSE_COM, bit == 0U);
	//GPIO_WRITE(TEST_PIN_REVERSE_COM, FALSE);
	//GPIO_WRITE(TEST_PIN_REVERSE_COM, TRUE);
}
