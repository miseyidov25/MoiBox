#include "HAL/hal.h"

#include "HAL/Input/keypad.h"
#include "HAL/Input/buttons.h"
#include "HAL/Audio/buzzer.h"
#include "HAL/Display/oled.h"
#include "HAL/Display/leds.h"
#include "HAL/Actuators/lock.h"
#include "HAL/BT/bt.h"

void hal_init(void)
{
    oled_init();
    leds_init();
    keypad_init();
    buttons_init();
    buzzer_init();
    lock_init();
    bt_init();
}

void hal_update(uint32_t current_ms)
{
    keypad_update(current_ms);
    buttons_update();
    bt_update();
    lock_update(current_ms);
}