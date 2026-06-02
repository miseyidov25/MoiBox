#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

void buttons_init(void);
void buttons_update(void);

bool buttons_red_pressed(void);
bool buttons_green_pressed(void);
bool buttons_blue_pressed(void);
bool buttons_yellow_pressed(void);

bool buttons_is_pressed(void);

#endif