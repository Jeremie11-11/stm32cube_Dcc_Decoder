/*
 * counter.c
 *
 *  Created on: Oct 18, 2024
 *      Author: J. Früh
 */


#include <m_counter.h>

COUNTERS_STRUCT Cnt;

// Use to start a counter
void cnt_start(counters_enum cnt_idx, uint32_t time_ms)
{
  if(cnt_idx < NBR_COUNTERS) {
      __disable_irq();  // Disable interrupts
      Cnt.time[cnt_idx] = time_ms;
      __enable_irq();   // Re-enable interrupts
  }
}

// Use to re-start a counter only if smaller
void cnt_start_if_not(counters_enum cnt_idx, uint32_t time_ms)
{
  if(cnt_idx < NBR_COUNTERS) {
      __disable_irq();  // Disable interrupts
      if(Cnt.time[cnt_idx] < time_ms)
      	Cnt.time[cnt_idx] = time_ms;
      __enable_irq();   // Re-enable interrupts
  }
}

// Read the remaining time
uint32_t cnt_read(counters_enum cnt_idx)
{
  uint32_t result = 0;

  if (cnt_idx < NBR_COUNTERS) {
      __disable_irq();  // Disable interrupts
      result = Cnt.time[cnt_idx];
      __enable_irq();   // Re-enable interrupts
  }
  return result;
}

uint32_t cnt_is_done(counters_enum cnt_idx)
{
  uint32_t result = 0;

  if (cnt_idx < NBR_COUNTERS) {
      __disable_irq();  // Disable interrupts
      if (Cnt.time[cnt_idx] == 0)
      	result = TRUE;
      else
      	result = FALSE;
      __enable_irq();   // Re-enable interrupts
  }
  return result;
}


// Updated in SysTick interrupt (each 1 ms)
void cnt_update()
{
	uint32_t i;

	for(i=0; i<NBR_COUNTERS; i++)
	{
		if(Cnt.time[i] > 0)
			Cnt.time[i]--;
		else
			Cnt.time[i] = 0;
	}
}
