#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void buzzer_init(void);
void buzzer_on(void);
void buzzer_off(void);

void buzzer_beep(uint32_t duration_ms);
void buzzer_morse_S(void);

void buzzer_success(void);
void buzzer_fail(void);
void buzzer_click(void);

#endif