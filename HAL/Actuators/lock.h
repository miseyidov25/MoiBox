#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>

void lock_init(void);
void lock_update(uint32_t current_ms);

void lock_lock(void);
void lock_unlock(void);

int lock_is_unlocked(void);

#endif