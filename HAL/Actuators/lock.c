#include "HAL/Actuators/lock.h"

static int unlocked = 0;

void lock_init(void)
{
    unlocked = 0;
}

void lock_lock(void)
{
    unlocked = 0;
}

void lock_unlock(void)
{
    unlocked = 1;
}

int lock_is_unlocked(void)
{
    return unlocked;
}