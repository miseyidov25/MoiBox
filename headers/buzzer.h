#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

void buzzer_init(void);
void buzzer_play_morse(const char *text); // basic A–Z morse encoding

#endif // BUZZER_H
