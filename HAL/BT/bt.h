#ifndef BT_H
#define BT_H

#include <stdbool.h>
#include "App/events.h"

void bt_init(void);
void bt_update(void);
bool bt_get_event(app_event_t *event);

#endif