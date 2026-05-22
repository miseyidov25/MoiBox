#include "App/settings.h"

static app_difficulty_t current_difficulty = APP_DIFFICULTY_EASY;
static app_language_t current_language = APP_LANGUAGE_ENGLISH;

void app_settings_init(void)
{
    current_difficulty = APP_DIFFICULTY_EASY;
    current_language = APP_LANGUAGE_ENGLISH;
}

void app_settings_set_difficulty(app_difficulty_t difficulty)
{
    if (difficulty == APP_DIFFICULTY_EASY ||
        difficulty == APP_DIFFICULTY_HARD)
    {
        current_difficulty = difficulty;
    }
}

app_difficulty_t app_settings_get_difficulty(void)
{
    return current_difficulty;
}

void app_settings_set_language(app_language_t language)
{
    if (language == APP_LANGUAGE_ENGLISH ||
        language == APP_LANGUAGE_DUTCH)
    {
        current_language = language;
    }
}

app_language_t app_settings_get_language(void)
{
    return current_language;
}

const char *app_settings_difficulty_to_string(app_difficulty_t difficulty)
{
    switch (difficulty)
    {
        case APP_DIFFICULTY_EASY:
            return "EASY";

        case APP_DIFFICULTY_HARD:
            return "HARD";

        default:
            return "UNKNOWN";
    }
}

const char *app_settings_language_to_string(app_language_t language)
{
    switch (language)
    {
        case APP_LANGUAGE_ENGLISH:
            return "ENGLISH";

        case APP_LANGUAGE_DUTCH:
            return "DUTCH";

        default:
            return "UNKNOWN";
    }
}