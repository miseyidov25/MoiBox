#include "HAL/Storage/logger.h"

#include "FatFS/ff.h"
#include "HAL/BT/hc05.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define LOG_FILE_NAME "MOIBOX.TXT"

static FATFS filesystem;

static bool logger_available = false;
static bool live_hc05_enabled = false;

static uint32_t last_ms = 0u;

static uint32_t run_number = 0u;
static uint32_t run_start_ms = 0u;
static bool run_active = false;

static uint32_t location_search_ms = 0u;
static uint32_t puzzle_start_ms = 0u;

static void uint_to_string(uint32_t value, char *out)
{
    char temp[11];
    uint32_t i = 0u;
    uint32_t j = 0u;

    if (value == 0u)
    {
        out[0] = '0';
        out[1] = '\0';
        return;
    }

    while ((value > 0u) && (i < 10u))
    {
        temp[i++] = (char)('0' + (value % 10u));
        value /= 10u;
    }

    while (i > 0u)
    {
        out[j++] = temp[--i];
    }

    out[j] = '\0';
}

static bool write_text(FIL *file, const char *text)
{
    UINT written = 0u;

    if (text == 0)
    {
        return true;
    }

    return f_write(file, text, (UINT)strlen(text), &written) == FR_OK;
}

static bool write_uint(FIL *file, uint32_t value)
{
    char text[12];

    uint_to_string(value, text);

    return write_text(file, text);
}

static void hc05_write_uint(uint32_t value)
{
    char text[12];

    uint_to_string(value, text);
    hc05_write_string(text);
}

static bool write_seconds_6_digits(FIL *file, uint32_t seconds)
{
    uint32_t divider = 100000u;
    uint32_t digit;
    char text[2];

    text[1] = '\0';

    while (divider > 0u)
    {
        digit = seconds / divider;
        text[0] = (char)('0' + digit);

        if (!write_text(file, text))
        {
            return false;
        }

        seconds %= divider;
        divider /= 10u;
    }

    return true;
}

static void hc05_write_seconds_6_digits(uint32_t seconds)
{
    uint32_t divider = 100000u;
    uint32_t digit;
    char text[2];

    text[1] = '\0';

    while (divider > 0u)
    {
        digit = seconds / divider;
        text[0] = (char)('0' + digit);

        hc05_write_string(text);

        seconds %= divider;
        divider /= 10u;
    }
}

static uint32_t run_time_seconds(uint32_t now_ms)
{
    if (!run_active)
    {
        return now_ms / 1000u;
    }

    return (now_ms - run_start_ms) / 1000u;
}

static bool open_log_file(FIL *file)
{
    FRESULT result;

    result = f_open(file, LOG_FILE_NAME, FA_WRITE | FA_OPEN_ALWAYS);

    if (result != FR_OK)
    {
        logger_available = false;
        return false;
    }

    result = f_lseek(file, f_size(file));

    if (result != FR_OK)
    {
        f_close(file);
        logger_available = false;
        return false;
    }

    return true;
}

static void close_log_file(FIL *file)
{
    if (f_close(file) != FR_OK)
    {
        logger_available = false;
    }
}

static bool write_prefix(FIL *file, uint32_t now_ms)
{
    if (!write_text(file, "[") ||
        !write_seconds_6_digits(file, run_time_seconds(now_ms)) ||
        !write_text(file, " s] RUN ") ||
        !write_uint(file, run_number) ||
        !write_text(file, " | "))
    {
        return false;
    }

    return true;
}

static void hc05_write_prefix(uint32_t now_ms)
{
    hc05_write_string("[");
    hc05_write_seconds_6_digits(run_time_seconds(now_ms));
    hc05_write_string(" s] RUN ");
    hc05_write_uint(run_number);
    hc05_write_string(" | ");
}

static void hc05_stream_basic_line(const char *category, const char *message, uint32_t now_ms)
{
    if (!live_hc05_enabled)
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

    hc05_write_prefix(now_ms);
    hc05_write_string(category);
    hc05_write_string(": ");
    hc05_write_string(message);
    hc05_write_string("\r\n");
}

static void hc05_stream_separator(const char *text, uint32_t now_ms)
{
    if (!live_hc05_enabled)
    {
        return;
    }

    hc05_write_prefix(now_ms);
    hc05_write_string("RUN: ========== ");
    hc05_write_string(text);
    hc05_write_string(" ==========\r\n");
}

static void hc05_stream_timing_line(
    const char *label,
    uint8_t number,
    const char *action,
    uint32_t now_ms,
    uint32_t duration_ms,
    bool has_duration
)
{
    if (!live_hc05_enabled)
    {
        return;
    }

    hc05_write_prefix(now_ms);
    hc05_write_string(label);
    hc05_write_string(" ");
    hc05_write_uint((uint32_t)number);
    hc05_write_string(" ");
    hc05_write_string(action);

    if (has_duration)
    {
        hc05_write_string(" after ");
        hc05_write_uint(duration_ms / 1000u);
        hc05_write_string(" s");
    }

    hc05_write_string("\r\n");
}

static void write_basic_line(const char *category, const char *message, uint32_t now_ms)
{
    FIL file;

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

    if (!open_log_file(&file))
    {
        return;
    }

    if (!write_prefix(&file, now_ms) ||
        !write_text(&file, category) ||
        !write_text(&file, ": ") ||
        !write_text(&file, message) ||
        !write_text(&file, "\r\n"))
    {
        f_close(&file);
        logger_available = false;
        return;
    }

    close_log_file(&file);

    hc05_stream_basic_line(category, message, now_ms);
}

static void write_separator(const char *text, uint32_t now_ms)
{
    FIL file;

    if (!logger_available)
    {
        return;
    }

    if (!open_log_file(&file))
    {
        return;
    }

    if (!write_prefix(&file, now_ms) ||
        !write_text(&file, "RUN: ========== ") ||
        !write_text(&file, text) ||
        !write_text(&file, " ==========\r\n"))
    {
        f_close(&file);
        logger_available = false;
        return;
    }

    close_log_file(&file);

    hc05_stream_separator(text, now_ms);
}

static void write_timing_line(
    const char *label,
    uint8_t number,
    const char *action,
    uint32_t now_ms,
    uint32_t duration_ms,
    bool has_duration
)
{
    FIL file;

    if (!logger_available)
    {
        return;
    }

    if (!open_log_file(&file))
    {
        return;
    }

    if (!write_prefix(&file, now_ms) ||
        !write_text(&file, label) ||
        !write_text(&file, " ") ||
        !write_uint(&file, (uint32_t)number) ||
        !write_text(&file, " ") ||
        !write_text(&file, action))
    {
        f_close(&file);
        logger_available = false;
        return;
    }

    if (has_duration)
    {
        if (!write_text(&file, " after ") ||
            !write_uint(&file, duration_ms / 1000u) ||
            !write_text(&file, " s"))
        {
            f_close(&file);
            logger_available = false;
            return;
        }
    }

    if (!write_text(&file, "\r\n"))
    {
        f_close(&file);
        logger_available = false;
        return;
    }

    close_log_file(&file);

    hc05_stream_timing_line(label, number, action, now_ms, duration_ms, has_duration);
}

void logger_init(void)
{
    FRESULT result;

    logger_available = false;
    live_hc05_enabled = false;

    run_active = false;
    run_number = 0u;
    run_start_ms = 0u;
    last_ms = 0u;
    location_search_ms = 0u;
    puzzle_start_ms = 0u;

    result = f_mount(&filesystem, "", 1);

    if (result == FR_OK)
    {
        logger_available = true;

        write_basic_line("SYSTEM", "LOGGER ON", last_ms);
        write_basic_line("SYSTEM", "NEW BOOT", last_ms);
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

void logger_log(const char *category, const char *message)
{
    write_basic_line(category, message, last_ms);
}

void logger_log_event(const char *event_name)
{
    write_basic_line("EVENT", event_name, last_ms);
}

void logger_log_settings(const char *message)
{
    write_basic_line("SETTINGS", message, last_ms);
}

void logger_log_input_char(char c)
{
    char msg[2];

    if ((c == '\r') || (c == '\n'))
    {
        return;
    }

    if (!(((c >= '0') && (c <= '9')) || (c == '*') || (c == '#')))
    {
        return;
    }

    msg[0] = c;
    msg[1] = '\0';

    write_basic_line("INPUT", msg, last_ms);
}

void logger_start_run(void)
{
    if (run_active)
    {
        return;
    }

    run_number++;
    run_active = true;
    run_start_ms = last_ms;

    write_separator("RUN START", last_ms);
}

void logger_end_run(void)
{
    uint32_t duration_ms;

    if (!run_active)
    {
        return;
    }

    duration_ms = last_ms - run_start_ms;

    write_timing_line("RUN", (uint8_t)run_number, "COMPLETE", last_ms, duration_ms, true);

    write_separator("RUN COMPLETE", last_ms);

    run_active = false;
}

void logger_location_search(uint8_t location, uint32_t now_ms)
{
    location_search_ms = now_ms;

    write_timing_line("LOCATION", location, "SEARCH", now_ms, 0u, false);
}

void logger_location_found(uint8_t location, uint32_t now_ms)
{
    uint32_t duration_ms = 0u;

    if (location_search_ms <= now_ms)
    {
        duration_ms = now_ms - location_search_ms;
    }

    write_timing_line("LOCATION", location, "FOUND", now_ms, duration_ms, true);
}

void logger_puzzle_start(uint8_t puzzle, uint32_t now_ms)
{
    puzzle_start_ms = now_ms;

    write_timing_line("PUZZLE", puzzle, "START", now_ms, 0u, false);
}

void logger_puzzle_done(uint8_t puzzle, uint32_t now_ms)
{
    uint32_t duration_ms = 0u;

    if (puzzle_start_ms <= now_ms)
    {
        duration_ms = now_ms - puzzle_start_ms;
    }

    write_timing_line("PUZZLE", puzzle, "DONE", now_ms, duration_ms, true);
}

void logger_puzzle_skip(uint8_t puzzle, uint32_t now_ms)
{
    uint32_t duration_ms = 0u;

    if (puzzle_start_ms <= now_ms)
    {
        duration_ms = now_ms - puzzle_start_ms;
    }

    write_timing_line("PUZZLE", puzzle, "SKIPPED", now_ms, duration_ms, true);
}

void logger_set_unix_time(uint32_t unix_time_seconds, uint32_t current_ms)
{
    (void)unix_time_seconds;

    last_ms = current_ms;

    write_basic_line("TIME", "SYNC", last_ms);
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
    (void)year;
    (void)month;
    (void)day;
    (void)hour;
    (void)minute;
    (void)second;

    logger_set_unix_time(0u, current_ms);
}

void logger_set_live_hc05(bool enabled)
{
    live_hc05_enabled = enabled;

    if (enabled)
    {
        hc05_write_string("\r\n===== LIVE LOG ON =====\r\n");
    }
    else
    {
        hc05_write_string("\r\n===== LIVE LOG OFF =====\r\n");
    }
}

bool logger_get_live_hc05(void)
{
    return live_hc05_enabled;
}

void logger_send_log_hc05(void)
{
    FIL file;
    FRESULT result;
    UINT bytes_read;
    char buffer[65];

    if (!logger_available)
    {
        hc05_write_string("\r\nLOG ERROR: SD logger unavailable\r\n");
        return;
    }

    result = f_open(&file, LOG_FILE_NAME, FA_READ);

    if (result != FR_OK)
    {
        hc05_write_string("\r\nLOG ERROR: cannot open MOIBOX.TXT\r\n");
        return;
    }

    hc05_write_string("\r\n===== PAST LOG START =====\r\n");

    do
    {
        bytes_read = 0u;

        result = f_read(&file, buffer, 64u, &bytes_read);

        if (result != FR_OK)
        {
            f_close(&file);
            hc05_write_string("\r\nLOG ERROR: read failed\r\n");
            return;
        }

        buffer[bytes_read] = '\0';

        if (bytes_read > 0u)
        {
            hc05_write_string(buffer);
        }

    } while (bytes_read > 0u);

    f_close(&file);

    hc05_write_string("\r\n===== PAST LOG END =====\r\n");
}