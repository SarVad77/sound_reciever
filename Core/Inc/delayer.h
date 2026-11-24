#ifndef _DELAYER
#define _DELAYER

#include "stm32f4xx_hal.h"



/* Delayer struct */

typedef struct
{
    uint32_t start_time;
    uint32_t sleep_time;
} delay_t;


/* Set delay point */
void set_delay(delay_t *delay, uint32_t i_sleep_time);

/* Reset delay point - status always will be 1 */
void reset_delay(delay_t *delay);

/* 
   Get delay status 
   Returns: 0 - delay time is not reached limit set, 1 - Everything is OK
*/
int8_t get_delay(delay_t *delay);

#endif
