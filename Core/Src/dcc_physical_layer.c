/*
 * dcc_protocol.c
 *
 *  Created on: Mar 21, 2024
 *      Author: J. Früh
 */

#include <dcc_physical_layer.h>
#include <i_dma.h>


DCC_PROTOCOL_STRUCT DccTx;
DCC_PROTOCOL_STRUCT DccRx;

extern DMA_DCC_STRUCT Dma;


DCC_DEBUG_STRUCT DccDebug;


inline static void dcc_tx_set_zero(void)
{
	TIM1->ARR = 888;
	TIM1->CCR1 = 444;
}


inline static void dcc_tx_set_one(void)
{
	TIM1->ARR = 514;
	TIM1->CCR1 = 257;
}


void dcc_init(void)
{
	DccDebug.recieved_msg = 0;

}


void dcc_tx_update(void)
{
	uint8_t val;

	if(DccTx.preamble_i > 0)
	{
		// ----- Preamble -----
		DccTx.preamble_i--;
		DccTx.bit_i = 255;
		DccTx.byte_i = 0;

		dcc_tx_set_one();
	}
	else
	{
		if(DccTx.bit_i >= 8)
		{

			// Next byte of the message
			if(DccTx.bit_i != 255)
				DccTx.byte_i++;

			if(DccTx.byte_i >= DccTx.msg[DccTx.msg_out_i].len)
			{
				// ----- End of message -----
				DccTx.preamble_i = 86;


				if(DccTx.sent_nbr > 0)
					// Repeat the same message
					DccTx.sent_nbr--;
				else
				{
					// Check for the next message
					if(DccTx.msg_out_i != DccTx.msg_in_i)
					{
						// ----- Get next queued message -----
						DccTx.msg[DccTx.msg_out_i].len = 0;

						DccTx.msg_out_i = (DccTx.msg_out_i + 1) & (DCC_MAX_MESSAGES_QUEUE-1);
					}

					DccTx.sent_nbr = DCC_MSG_REPEATED_TIME;
				}

				// End bit
				dcc_tx_set_one();
			}
			else
			{
				// ----- Next byte -----
				DccTx.bit_i = 0;

				// Start bit
				dcc_tx_set_zero();
			}
		}
		else
		{
			// ----- Next bit of the byte -----
			val = (DccTx.msg[DccTx.msg_out_i].data[DccTx.byte_i] << DccTx.bit_i) & 0x80;

			if(val == 0)
				dcc_tx_set_zero();
			else
				dcc_tx_set_one();

			DccTx.bit_i++;
		}


	}
}


void dcc_rx_update(void)
{
	uint32_t val;
	static uint32_t val1;
	static uint8_t byte, byte_xor;

	while(Dma.idx_out0 != Dma.idx_in)
	{
		val = Dma.time_buffer[Dma.idx_out0] + Dma.time_buffer[Dma.idx_out1];

		if(DccRx.preamble_i > 1)
		{
			// ----- Preamble (10 bits) -----
			if((val >= DCC_RX_ONE_LOW) && (val <= DCC_RX_ONE_HIGH))
				DccRx.preamble_i--;
			else
				DccRx.preamble_i = DCC_RX_PREAMBLE_INIT;

		}
		else if(DccRx.preamble_i > 0)
		{
			// ----- Waiting for start bit -----
			if((val >= DCC_RX_ONE_LOW) && (val <= DCC_RX_ONE_HIGH))
			{
				// Keep waiting
			}
			else if((val >= DCC_RX_ZERO_LOW) && (val <= DCC_RX_ZERO_HIGH))
			{
				// Start bit received
				DccRx.preamble_i = 0;
				DccRx.bit_i = 2;
				DccRx.byte_i = 0;
				byte = 0x00;
				byte_xor = 0x00;

				DccDebug.buffer[0] = val;
				DccDebug.idx = 1;
			}
			else if((val >= DCC_RX_ONE_LOW) && (val <= DCC_RX_ZERO_HIGH) && (val1 >= DCC_RX_ONE_LOW) && (val1 <= DCC_RX_ONE_HIGH))
			{
				// Keep waiting
			}
			else
			{
				// Invalid
				DccRx.preamble_i = DCC_RX_PREAMBLE_INIT;
				DccDebug.first_start_error++;

			  dbg_set_error_led(LED_DCC_COM_ERRORS, 20);
			}
		}
		else
		{
			if((DccRx.bit_i & 0x01) == 1)
			{
				DccDebug.buffer[DccDebug.idx++] = val;
				if(val<30)
					DccDebug.val_buffer[val]++;

				if(!(((val >= DCC_RX_ZERO_LOW) && (val <= DCC_RX_ZERO_HIGH)) || ((val >= DCC_RX_ONE_LOW) && (val <= DCC_RX_ONE_HIGH))))
				{
					// Invalid bit size
					DccRx.preamble_i = DCC_RX_PREAMBLE_INIT;
					DccDebug.bit_size_error++;

				  dbg_set_error_led(LED_DCC_COM_ERRORS, 20);
				}
				else if(DccRx.bit_i == 1)
				{
					DccRx.msg[DccRx.msg_in_i].data[DccRx.byte_i] = byte;
					byte_xor = byte_xor^byte;
					byte = 0x00;

					DccRx.byte_i++;
					if(DccRx.byte_i >= DCC_MAX_MESSAGE_LEN)
					{
						DccRx.preamble_i = DCC_RX_PREAMBLE_INIT;
						DccDebug.msg_len_error++;

					  dbg_set_error_led(LED_DCC_COM_ERRORS, 20);
					}

					if((val >= DCC_RX_ZERO_LOW) && (val <= DCC_RX_ZERO_HIGH))
					{
						// Start bit
					}
					else
					{
						// ----- Stop bit received -----
						if(byte_xor != 0x00)
						{
							// ----- XOR byte do not match error -----
							DccRx.preamble_i = DCC_RX_PREAMBLE_INIT;
							DccDebug.xor_error++;

							dbg_set_error_led(LED_DCC_COM_ERRORS, 20);
						}
						else if(DccRx.byte_i < 3)
						{
							// ----- Message length error -----
							DccRx.preamble_i = DCC_RX_PREAMBLE_INIT;
							DccDebug.msg_len_error++;

							dbg_set_error_led(LED_DCC_COM_ERRORS, 20);
						}
						else
						{
							// ----- New message received correctly -----
							DccDebug.recieved_msg++;

							DccRx.msg[DccRx.msg_in_i].len = DccRx.byte_i;
							DccRx.msg_in_i = (DccRx.msg_in_i + 1) & (DCC_MAX_MESSAGES_QUEUE-1);
							DccRx.preamble_i = DCC_RX_PREAMBLE_INIT;
						}
					}
				}
				else
				{
					if((val >= DCC_RX_ONE_LOW) && (val <= DCC_RX_ONE_HIGH))
					{
						byte = (byte<<1) + 1;
					}
					else
					{
						byte = (byte<<1);
					}
				}

			}

			DccRx.bit_i++;
			if(DccRx.bit_i >= 18)
				DccRx.bit_i = 0;
		}

		val1 = val;
		Dma.idx_out1 = Dma.idx_out0;
		Dma.idx_out0 = (Dma.idx_out0+1) & DMA_TIME_IDX_MASK;
	}
}


