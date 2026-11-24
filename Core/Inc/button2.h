#ifndef _BUTTON2_H
#define _BUTTON2_H

#include "stm32f4xx_hal.h"

// Maximum time between multiply clicks (milliseconds)
#define BUTTON_PAUSE_TIME 150

// Time for hold state (milliseconds)
#define BUTTON_HOLD_TIME 500

// Time of last idle/unpressed button state (milliseconds) - then release button handler function
#define BUTTON_FREE 300

// Limit of multiply clicks
#define MAX_PRESSED_COUNT 127

// Drebezzhal tramway w shkafu
#define DEBOUNCE_TIME 30

// Button states
#define RB_IDLE 0        // Button is free
#define RB_INPROGRESS 1  // Button is pressed and not finished yet
#define RB_RESULT 2      // Button pressing has been finished. Read result and flush settings for further pressing

#define RB_HOLD 0xFF     // Button is hold

/* Button struct */

typedef struct
{
    // Hardware
    GPIO_TypeDef *h_button_port;
    uint16_t h_button_pin;

    // Software / Settings
    uint16_t s_button_pause_time;
    uint16_t s_button_hold_time;
    uint16_t s_button_free;
    uint8_t s_max_pressed_count;
    uint8_t s_debounce_time;

    // Runtime
    uint8_t r_first_start;
    uint8_t r_pressed;
    uint8_t r_busy_state;
    uint8_t r_pressed_count;
    uint32_t r_last_event;


} button_t;


/*
   Is button pressed funxtion
   Results: 0 - unpressed, 1 - pressed
*/
uint8_t button_pressed(button_t *b);

/*
  Init function for button structure - set defaults
*/
void init_button(button_t *b);

/*
  Button handler function - called by interrupt
*/

void button_irq_handler(button_t *b, uint8_t pressed);

/*
  Update button status
*/
void button_query(button_t *b);

/*
  Take result and set button free
*/
uint8_t button_result(button_t *b);

#endif
