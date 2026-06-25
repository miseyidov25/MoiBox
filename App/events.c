#include "App/events.h"

static const char *eventEnumToText[] =
{
    "EVENT_NONE",

    "EVENT_BEACON_1_DETECTED",
    "EVENT_BEACON_2_DETECTED",
    "EVENT_BEACON_3_DETECTED",
    "EVENT_BEACON_4_DETECTED",
    "EVENT_BEACON_5_DETECTED",

    "EVENT_KEYPAD_KEY",

    "EVENT_BUTTON_RED",
    "EVENT_BUTTON_GREEN",
    "EVENT_BUTTON_BLUE",
    "EVENT_BUTTON_YELLOW",

    "EVENT_SKIP_REQUEST",
    "EVENT_RESET_REQUEST",
    "EVENT_OPEN_REQUEST"
};

#define NUM_EVENTS (sizeof(eventEnumToText) / sizeof(eventEnumToText[0]))

const char *event_to_string(app_event_type_t event)
{
    if (((int)event >= 0) && ((unsigned int)event < NUM_EVENTS))
    {
        return eventEnumToText[event];
    }

    return "UNKNOWN_EVENT";
}