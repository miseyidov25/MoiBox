#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

/*
 * Hardware enable flags.
 */
#define APP_KEYPAD_ENABLED   1
#define APP_BUTTONS_ENABLED  1
#define APP_BT_ENABLED       1
#define APP_SERIAL_TESTING   1

typedef enum
{
    APP_DIFFICULTY_EASY = 0,
    APP_DIFFICULTY_HARD
} app_difficulty_t;

typedef enum
{
    APP_LANGUAGE_ENGLISH = 0,
    APP_LANGUAGE_DUTCH
} app_language_t;

void app_settings_init(void);

void app_settings_set_difficulty(app_difficulty_t difficulty);
app_difficulty_t app_settings_get_difficulty(void);

void app_settings_set_language(app_language_t language);
app_language_t app_settings_get_language(void);

const char *app_settings_difficulty_to_string(app_difficulty_t difficulty);
const char *app_settings_language_to_string(app_language_t language);

#endif