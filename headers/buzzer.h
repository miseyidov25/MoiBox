#ifndef BUZZER_H
#define BUZZER_H


void buzzer_init(void);
void buzzer_play_morse(const char *text); // basic A–Z morse encoding
// other tones (success, error, beep) can be added later

#endif // BUZZER_H
