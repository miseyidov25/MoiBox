#include "HAL/hal.h"

#include "App/settings.h"

#include "HAL/Display/oled.h"
#include "HAL/Display/leds.h"

#include "HAL/Input/keypad.h"
#include "HAL/Input/buttons.h"

#include "HAL/Actuators/lock.h"
#include "HAL/Audio/buzzer.h"

#include "HAL/BT/bt.h"
#include "HAL/BT/hc05.h"

void hal_init(void)
{
    oled_init();

    leds_init();

    keypad_init();
    buttons_init();

    lock_init();
    buzzer_init();

#if APP_BT_ENABLED
    bt_init();
    hc05_init(9600u);
#endif
}

void hal_update(uint32_t current_ms)
{
    leds_update_map(0u, 0);

    keypad_update(current_ms);
    buttons_update();

    buzzer_update(current_ms);

#if APP_BT_ENABLED
    bt_update();
    hc05_update();
#endif
}