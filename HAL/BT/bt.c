#include "HAL/BT/bt.h"

void bt_init(void)
{
}

void bt_update(void)
{
}

bool bt_get_event(app_event_t *event)
{
    if (event != 0)
    {
        event->type = EVENT_NONE;
        event->keypad_key = '\0';
        event->rssi = 0;
    }

    return false;
}