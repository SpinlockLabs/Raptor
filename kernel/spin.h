#pragma once

#include <stdint.h>

#include <liblox/atomic.h>

/**
 * Spin lock mechanism.
 */
typedef struct spin_lock {
    atomic_int32 addr;
    atomic_int32 waiters;
} spin_lock_t;

/**
 * Initialize the spinlock.
 * @param lock lock instance
 */
extern void spin_init(spin_lock_t* lock);

/**
 * Locks a spinlock.
 * @param lock lock instance
 */
extern void spin_lock(spin_lock_t* lock);

/**
 * Unlocks a spinlock.
 * @param lock lock instance
 */
extern void spin_unlock(spin_lock_t* lock);
