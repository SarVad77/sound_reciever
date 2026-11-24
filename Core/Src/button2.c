#include "button2.h"
#include "main.h"


// Is button pressed
uint8_t button_pressed(button_t *b)
{
    return (HAL_GPIO_ReadPin(b->h_button_port, b->h_button_pin) == GPIO_PIN_RESET);
}

void init_button(button_t *b)
{
        
    // Software / Settings
    b->s_button_pause_time = BUTTON_PAUSE_TIME;
    b->s_button_hold_time = BUTTON_HOLD_TIME;
    b->s_button_free = BUTTON_FREE;
    b->s_max_pressed_count = MAX_PRESSED_COUNT;
    b->s_debounce_time = DEBOUNCE_TIME;

    // Runtime
    b->r_first_start = 1;
    b->r_pressed = 0;
    b->r_busy_state = 0;
    b->r_pressed_count = 0;
    b->r_last_event = 0;


}

void button_irq_handler(button_t *b, uint8_t pressed)
{

	// Bypass handler
	if (!b->h_button_port || !b->h_button_pin || !b->r_first_start || b->r_busy_state == RB_RESULT)
		return;

	uint32_t now = HAL_GetTick();
	

	if (pressed) {
		if (!b->r_busy_state || now - b->r_last_event >= b->s_debounce_time)
		{


			if ((!b->r_busy_state || now - b->r_last_event <= b->s_button_pause_time) && b->r_pressed_count < b->s_max_pressed_count)
				b->r_pressed_count++;

			b->r_busy_state = RB_INPROGRESS; // First strike

			b->r_last_event = now;
			b->r_pressed = pressed;

		}

	}
	else
	{
		if (now - b->r_last_event >= b->s_button_hold_time && b->r_busy_state==RB_INPROGRESS)
		{
			b->r_pressed_count = RB_HOLD;
			b->r_busy_state = RB_RESULT;
		}
		b->r_last_event = now;
		b->r_pressed = pressed;

	}

}

void button_query(button_t *b)
{
     uint32_t now = HAL_GetTick();


   	 if (b->r_first_start && !b->r_pressed && b->r_busy_state == RB_INPROGRESS && now - b->r_last_event > b->s_button_free) // Result is available
     {
          b->r_busy_state = RB_RESULT;
     }
}

uint8_t button_result(button_t *b)
{
	uint8_t last_result;
	if (b->r_first_start && b->r_busy_state == RB_RESULT)
	{
		last_result = b->r_pressed_count;
		b->r_busy_state = RB_IDLE;
		b->r_last_event = 0;
		b->r_pressed_count = 0;
		b->r_pressed = 0;

		return last_result;
	}

	return 0;
}

/*void button_status(button_t *b, uint8_t state)
{
	uint8_t prev_state = 0;    // Previous state
	uint8_t pressed_count = 0; // How many clicks captured

	uint32_t now = 0;          // Current time
	uint32_t prev_time = 0;    // Previous time if state changed
	uint32_t last_idle = 0;    // Time of button inactivity - needs to release button handler
	uint8_t fsid = 0;          // First usage of button handler, then sets to 1


	prev_state = 0;
 	pressed_count = 0;
 	last_idle = 0;
 	fsid = 0;
 	prev_time = HAL_GetTick();

        now = HAL_GetTick();
        state = button_pressed(b); // 0 - button up, 1 - button down


        if (!fsid || state!=prev_state)
        {
        	prev_time = now;
        	fsid = 1;
        }


        if (state) // Button is pressed
        {
    	   if (state==prev_state)
    	   {
    		  if (now - prev_time > b->button_hold_time)
    		  {
    			  // Sets button state to hold and leave cycle
    			  pressed_count=-1;
    			  b->button_hold = 1;
    			  return;
    		  }

    	   }
    	   else
    	   {
    		if (now - prev_time <= b->button_pause_time && pressed_count < b->max_pressed_count)
                  pressed_count++;  // Sets count of button clicks
           }
           last_idle = 0;  // Reset idle time
        }
        else
        {
        	if (b->button_hold) // Mandatory set button status to "not pressed" after hold status
        	{
        	        	b->button_hold = 0;
        	         	pressed_count = 0;
        	         	return;
           	}
        	last_idle = now - prev_time;  // Increase idle time - needs to leave cycle
        }
        prev_state = state;  // Save previous state




}
*/
