#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

typedef enum
{
    EVENT_NONE = 0,

    EVENT_BEACON_1_DETECTED,
    EVENT_BEACON_2_DETECTED,
    EVENT_BEACON_3_DETECTED,
    EVENT_BEACON_4_DETECTED,
    EVENT_BEACON_5_DETECTED,

    EVENT_KEYPAD_KEY,

    EVENT_BUTTON_RED,
    EVENT_BUTTON_GREEN,
    EVENT_BUTTON_BLUE,
    EVENT_BUTTON_YELLOW,

    EVENT_SKIP_REQUEST,
    EVENT_RESET_REQUEST
} app_event_type_t;

typedef struct
{
    app_event_type_t type;
    char keypad_key;
    int8_t rssi;
} app_event_t;

const char *event_to_string(app_event_type_t event);

#endif