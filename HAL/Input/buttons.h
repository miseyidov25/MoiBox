#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

// Button events
typedef enum {
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_RELEASED
} button_event_t;

// Button callback type
typedef void (*button_callback_t)(button_event_t event);

// Function declarations
void buttons_init(void);
void buttons_poll(void);
void buttons_register_callback(button_callback_t callback);
bool buttons_is_pressed(void);

#endif // BUTTONS_H
