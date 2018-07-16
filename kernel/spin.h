#pragma once

#include <stdint.h>

#include <liblox/atomic.h>

/**
 * Spinlock mechanism.
 */
typedef struct spin_lock {
    atomic_int32 addr;
    atomic_int32 waiters;
    void* last_acquired_by;

#ifdef DEBUG_SPINLOCKS
    char* label;
#endif
} spin_lock_t;

// Makes it possible to mark a spin lock with a label.
#ifdef DEBUG_SPINLOCKS
#define SET_SPIN_LOCK_LABEL(lock, name) (lock)->label = (name)
#else
#define SET_SPIN_LOCK_LABEL(lock, name)
#endif

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
