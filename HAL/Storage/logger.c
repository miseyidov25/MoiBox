#include "HAL/Storage/logger.h"

#include "FatFS/ff.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LOG_FILE_NAME "MOIBOX_LOG.TXT"
#define LOG_LINE_MAX  160u

static FATFS filesystem;

static bool logger_available = false;
static bool logger_mounted = false;

static bool unix_time_valid = false;
static uint32_t unix_time_base_seconds = 0u;
static uint32_t unix_time_base_ms = 0u;

static uint32_t last_ms = 0u;

static uint32_t current_unix_time(void)
{
    if (!unix_time_valid)
    {
        return 0u;
    }

    return unix_time_base_seconds + ((last_ms - unix_time_base_ms) / 1000u);
}

static void append_line_to_sd(const char *line)
{
    FIL file;
    FRESULT result;
    UINT written = 0u;

    if (!logger_available)
    {
        return;
    }

    result = f_open(&file, LOG_FILE_NAME, FA_WRITE | FA_OPEN_ALWAYS);

    if (result != FR_OK)
    {
        logger_available = false;
        return;
    }

    result = f_lseek(&file, f_size(&file));

    if (result != FR_OK)
    {
        f_close(&file);
        logger_available = false;
        return;
    }

    result = f_write(&file, line, (UINT)strlen(line), &written);

    if (result != FR_OK)
    {
        f_close(&file);
        logger_available = false;
        return;
    }

    result = f_sync(&file);

    if (result != FR_OK)
    {
        f_close(&file);
        logger_available = false;
        return;
    }

    f_close(&file);
}

void logger_init(void)
{
    FRESULT result;

    logger_available = false;
    logger_mounted = false;

    result = f_mount(&filesystem, "", 1);

    if (result != FR_OK)
    {
        /*
         * SD card not available.
         * Logging disabled silently.
         */
        return;
    }

    logger_mounted = true;
    logger_available = true;

    logger_log("SYSTEM", "Logger started");
}

void logger_update(uint32_t current_ms)
{
    last_ms = current_ms;
}

bool logger_is_available(void)
{
    return logger_available;
}

void logger_set_unix_time(uint32_t unix_time_seconds, uint32_t current_ms)
{
    unix_time_base_seconds = unix_time_seconds;
    unix_time_base_ms = current_ms;
    unix_time_valid = true;

    logger_log("TIME", "Unix time synchronized from app");
}

void logger_log(const char *category, const char *message)
{
    char line[LOG_LINE_MAX];

    if (!logger_available)
    {
        return;
    }

    if (category == 0)
    {
        category = "LOG";
    }

    if (message == 0)
    {
        message = "";
    }

    if (unix_time_valid)
    {
        snprintf(
            line,
            sizeof(line),
            "[%010lu ms] [unix=%lu] %s: %s\r\n",
            (unsigned long)last_ms,
            (unsigned long)current_unix_time(),
            category,
            message
        );
    }
    else
    {
        snprintf(
            line,
            sizeof(line),
            "[%010lu ms] %s: %s\r\n",
            (unsigned long)last_ms,
            category,
            message
        );
    }

    append_line_to_sd(line);
}

void logger_log_event(const char *event_name)
{
    logger_log("EVENT", event_name);
}

void logger_log_settings(const char *message)
{
    logger_log("SETTINGS", message);
}

void logger_log_input_char(char input)
{
    char message[32];

    if (!logger_available)
    {
        return;
    }

    if (input == '\r')
    {
        logger_log("INPUT", "\\r");
    }
    else if (input == '\n')
    {
        logger_log("INPUT", "\\n");
    }
    else
    {
        snprintf(message, sizeof(message), "char=%c", input);
        logger_log("INPUT", message);
    }
}