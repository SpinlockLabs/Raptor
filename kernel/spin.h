#pragma once

#include <stdint.h>
#include <liblox/atomic.h>

typedef struct spin_lock {
    atomic_int addr;
    atomic_int waiters;
} spin_lock_t;

extern void spin_init(spin_lock_t* lock);
extern void spin_lock(spin_lock_t* lock);
extern void spin_unlock(spin_lock_t* lock);
