#ifndef LOCK_H
#define LOCK_H

void lock_init(void);
void lock_lock(void);
void lock_unlock(void);
int lock_is_unlocked(void);

#endif