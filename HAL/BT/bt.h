#ifndef BT_H
#define BT_H

#include <stdint.h>
#include <stdbool.h>

#include "App/events.h"

void bt_init(void);
void bt_update(void);

bool bt_get_event(app_event_t *event);

void bt_detect_beacon(uint16_t major, uint16_t minor, int rssi);

void bt_set_expected_minor_for_puzzle(uint8_t puzzle_number, uint16_t minor);
uint16_t bt_get_expected_minor_for_puzzle(uint8_t puzzle_number);

void bt_debug_simulate_minor(uint16_t minor);

void bt_set_scanning_enabled(bool enabled);
bool bt_is_scanning_enabled(void);

#endif