#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdbool.h>

void logger_init(void);
void logger_update(uint32_t current_ms);

bool logger_is_available(void);

void logger_log(const char *category, const char *message);
void logger_log_event(const char *event_name);
void logger_log_settings(const char *message);
void logger_log_input_char(char c);

void logger_start_run(void);
void logger_end_run(void);

/*
 * Optional PC/app time support.
 *
 * The board cannot know real time alone.
 * The PC/app must send the current time once.
 */
void logger_set_unix_time(uint32_t unix_time_seconds, uint32_t current_ms);

void logger_set_datetime(
    uint16_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second,
    uint32_t current_ms
);

#endif