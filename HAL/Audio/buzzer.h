#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

/*
 * Buzzer driver for active-buzzer style behaviour.
 *
 * Hardware:
 *   buzzer signal -> P3_31
 *   buzzer GND    -> GND
 *
 * Behaviour:
 *   P3_31 HIGH -> sound
 *   P3_31 LOW  -> silent
 */

void buzzer_init(void);
void buzzer_update(uint32_t current_ms);

void buzzer_on(void);
void buzzer_off(void);
void buzzer_beep(uint32_t duration_ms);

void buzzer_set(uint32_t frequency_hz);

void buzzer_correct_sound(void);
void buzzer_error_sound(void);

void buzzer_success(void);
void buzzer_fail(void);
void buzzer_click(void);

void buzzer_morse_digit(uint8_t digit);
void buzzer_morse_S(void);
void buzzer_morse_string(const char *text);

void buzzer_repeat_start(void);
void buzzer_repeat_stop(void);

#endif