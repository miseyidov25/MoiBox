#include "HAL/Storage/logger.h"

#include "FatFS/ff.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define LOG_FILE_NAME "MOIBOX_LOG.TXT"
#define LOG_LINE_MAX  180u

static FATFS filesystem;

static bool logger_available = false;
static bool logger_mounted = false;

static bool time_valid = false;
static uint32_t time_base_unix_seconds = 0u;
static uint32_t time_base_ms = 0u;

static uint32_t last_ms = 0u;

static uint32_t run_number = 0u;
static bool run_active = false;

static int is_leap_year(uint32_t year)
{
    if ((year % 400u) == 0u)
    {
        return 1;
    }

    if ((year % 100u) == 0u)
    {
        return 0;
    }

    if ((year % 4u) == 0u)
    {
        return 1;
    }

    return 0;
}

static uint32_t days_in_month(uint32_t year, uint32_t month)
{
    static const uint8_t normal_days[12] =
    {
        31u, 28u, 31u, 30u, 31u, 30u,
        31u, 31u, 30u, 31u, 30u, 31u
    };

    if ((month == 2u) && is_leap_year(year))
    {
        return 29u;
    }

    if ((month < 1u) || (month > 12u))
    {
        return 30u;
    }

    return normal_days[month - 1u];
}

static uint32_t datetime_to_unix(
    uint32_t year,
    uint32_t month,
    uint32_t day,
    uint32_t hour,
    uint32_t minute,
    uint32_t second
)
{
    uint32_t days = 0u;

    if (year < 1970u)
    {
        return 0u;
    }

    for (uint32_t y = 1970u; y < year; y++)
    {
        days += is_leap_year(y) ? 366u : 365u;
    }

    for (uint32_t m = 1u; m < month; m++)
    {
        days += days_in_month(year, m);
    }

    if (day > 0u)
    {
        days += day - 1u;
    }

    return (days * 86400u) + (hour * 3600u) + (minute * 60u) + second;
}

static uint32_t current_unix_time(void)
{
    if (!time_valid)
    {
        return 0u;
    }

    return time_base_unix_seconds + ((last_ms - time_base_ms) / 1000u);
}

static void unix_to_datetime(
    uint32_t unix_time,
    uint32_t *year,
    uint32_t *month,
    uint32_t *day,
    uint32_t *hour,
    uint32_t *minute,
    uint32_t *second
)
{
    uint32_t days;
    uint32_t seconds_of_day;
    uint32_t y;
    uint32_t m;

    days = unix_time / 86400u;
    seconds_of_day = unix_time % 86400u;

    *hour = seconds_of_day / 3600u;
    seconds_of_day %= 3600u;

    *minute = seconds_of_day / 60u;
    *second = seconds_of_day % 60u;

    y = 1970u;

    while (1)
    {
        uint32_t days_this_year = is_leap_year(y) ? 366u : 365u;

        if (days < days_this_year)
        {
            break;
        }

        days -= days_this_year;
        y++;
    }

    m = 1u;

    while (1)
    {
        uint32_t dim = days_in_month(y, m);

        if (days < dim)
        {
            break;
        }

        days -= dim;
        m++;
    }

    *year = y;
    *month = m;
    *day = days + 1u;
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

    if (result == FR_OK)
    {
        logger_mounted = true;
        logger_available = true;
    }

    logger_log("SYSTEM", "Logger initialized");
    logger_log("BUILD", "Firmware build " __DATE__ " " __TIME__);

    if (logger_available)
    {
        logger_log("STORAGE", "SD logging enabled");
    }
    else
    {
        /*
         * SD unavailable.
         * No file logging possible.
         */
    }
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
    time_base_unix_seconds = unix_time_seconds;
    time_base_ms = current_ms;
    time_valid = true;

    logger_log("TIME", "PC/app time synchronized");
}

void logger_set_datetime(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second,
    uint32_t current_ms
)
{
    uint32_t unix_time;

    unix_time = datetime_to_unix(
        (uint32_t)year,
        (uint32_t)month,
        (uint32_t)day,
        (uint32_t)hour,
        (uint32_t)minute,
        (uint32_t)second
    );

    logger_set_unix_time(unix_time, current_ms);
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

    if (time_valid)
    {
        uint32_t y;
        uint32_t mo;
        uint32_t d;
        uint32_t h;
        uint32_t mi;
        uint32_t s;

        unix_to_datetime(current_unix_time(), &y, &mo, &d, &h, &mi, &s);

        snprintf(
            line,
            sizeof(line),
            "[%04lu-%02lu-%02lu %02lu:%02lu:%02lu] RUN %lu | %s: %s\r\n",
            (unsigned long)y,
            (unsigned long)mo,
            (unsigned long)d,
            (unsigned long)h,
            (unsigned long)mi,
            (unsigned long)s,
            (unsigned long)run_number,
            category,
            message
        );
    }
    else
    {
        snprintf(
            line,
            sizeof(line),
            "[%06lu s] RUN %lu | %s: %s\r\n",
            (unsigned long)(last_ms / 1000u),
            (unsigned long)run_number,
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

void logger_log_input_char(char c)
{
    char msg[32];

    if ((c == '\r') || (c == '\n'))
    {
        return;
    }

    if (c == '#')
    {
        logger_log("INPUT", "key=# / ENTER");
    }
    else if (c == '*')
    {
        logger_log("INPUT", "key=* / BACKSPACE");
    }
    else if ((c >= '0') && (c <= '9'))
    {
        msg[0] = 'k';
        msg[1] = 'e';
        msg[2] = 'y';
        msg[3] = '=';
        msg[4] = c;
        msg[5] = '\0';

        logger_log("INPUT", msg);
    }
    else
    {
        msg[0] = 'c';
        msg[1] = 'm';
        msg[2] = 'd';
        msg[3] = '=';
        msg[4] = c;
        msg[5] = '\0';

        logger_log("INPUT", msg);
    }
}

void logger_start_run(void)
{
    if (run_active)
    {
        return;
    }

    run_number++;
    run_active = true;

    logger_log("RUN", "========== RUN START ==========");
}

void logger_end_run(void)
{
    if (!run_active)
    {
        return;
    }

    logger_log("RUN", "========== RUN COMPLETE ==========");
    run_active = false;
}