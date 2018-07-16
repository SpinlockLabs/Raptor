#include "spin.h"
#include "panic.h"

#include <liblox/io.h>
#include <liblox/string.h>

#include <kernel/entry.h>

#include <kernel/cpu/task.h>
#include <kernel/interupt.h>

#ifndef ARCH_NO_SPINLOCK
void spin_wait(atomic_int32* addr, atomic_int32* waiters) {
    if (addr == NULL) {
        return;
    }

    if (waiters) {
        atomic_fetch_add(waiters, 1);
    }

    while (*addr) {
        if (kernel_is_initialized()) {
            ktask_queue_flush();
        } else {
            irq_wait();
        }
    }

    if (waiters) {
        atomic_fetch_sub(waiters, 1);
    }
}
#endif

void spin_lock(spin_lock_t* lock) {
#ifdef DEBUG_SPINLOCKS
    if (lock->label == NULL) {
        printf(DEBUG "[Kernel Locks] Acquired\n");
    } else {
        printf(DEBUG "[Kernel Locks] Acquired '%s'\n", lock->label);
    }
#endif

#ifdef ARCH_NO_SPINLOCK
    unused(lock);
    return;
#else
    bool warned = false;
    while (atomic_exchange(&lock->addr, 1)) {
        if (!warned) {
            warned = true;
#ifdef DEBUG_SPINLOCKS
            printf(WARN "[Kernel Locks] Waiting for lock... (%s)\n", lock->label);
#else
            printf(WARN "[Kernel Locks] Waiting for lock... (acquired by 0x%x)\n", lock->last_acquired_by);
#endif
        }
        spin_wait(&lock->addr, &lock->waiters);
    }
#endif

    lock->last_acquired_by = __builtin_return_address(1);
}

void spin_init(spin_lock_t* lock) {
#ifdef DEBUG_SPINLOCKS
    printf(DEBUG "[Kernel Locks] Init\n");
#endif
    memset(lock, 0, sizeof(spin_lock_t));
}

void spin_unlock(spin_lock_t* lock) {
#ifdef DEBUG_SPINLOCKS
    if (lock->label == NULL) {
        printf(DEBUG "[Kernel Locks] Released\n");
    } else {
        printf(DEBUG "[Kernel Locks] Released '%s'\n", lock->label);
    }
#endif

#ifdef ARCH_NO_SPINLOCK
    unused(lock);
    return;
#else
    if (lock->addr) {
        atomic_store(&lock->addr, 0);
        if (lock->waiters && kernel_is_initialized()) {
            ktask_queue_flush();
        }
    }
#endif
}
