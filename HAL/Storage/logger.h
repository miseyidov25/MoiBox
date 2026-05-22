#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdbool.h>

void logger_init(void);
void logger_update(uint32_t current_ms);

bool logger_is_available(void);

void logger_log(const char *category, const char *message);
void logger_log_event(const char *event_name);
void logger_log_input_char(char input);
void logger_log_settings(const char *message);

void logger_set_unix_time(uint32_t unix_time_seconds, uint32_t current_ms);

#endif