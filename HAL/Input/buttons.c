#include "buttons.h"
#include <stddef.h>
#include <MCXA153.h>

// Button configuration
#define BUTTON_GPIO        GPIO3
#define BUTTON_PIN         29U

// Button state tracking
static bool button_last_state = false;
static button_callback_t button_callback = NULL;

// Initialize button GPIO
void buttons_init(void)
{
    PORT3->PCR[BUTTON_PIN] = PORT_PCR_LK(1) |
                             PORT_PCR_MUX(1) |
                             PORT_PCR_PE(1) |
                             PORT_PCR_PS(1) |
                             PORT_PCR_IBE(1);
    BUTTON_GPIO->PDDR &= ~(1U << BUTTON_PIN);
    button_last_state = false;
}

// Check if button is pressed
static bool button_pressed(void)
{
    return (BUTTON_GPIO->PDR[BUTTON_PIN] == 0U);
}

//Poll button and call callback on state change
void buttons_poll(void)
{
    bool button_now = button_pressed();
    
    if (button_now && !button_last_state)
    {
        // Button pressed
        if (button_callback)
        {
            button_callback(BUTTON_EVENT_PRESSED);
        }
    }
    else if (!button_now && button_last_state)
    {
        // Button released
        if (button_callback)
        {
            button_callback(BUTTON_EVENT_RELEASED);
        }
    }
    
    button_last_state = button_now;
}

// Register button event callback
void buttons_register_callback(button_callback_t callback)
{
    button_callback = callback;
}

//Get current button state
bool buttons_is_pressed(void)
{
    return button_pressed();
}
