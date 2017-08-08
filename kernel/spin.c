#include "spin.h"

#include <liblox/io.h>

#include <kernel/entry.h>

#include <kernel/cpu/task.h>
#include <kernel/interupt.h>
#include <liblox/string.h>

void spin_wait(atomic_int32* addr, atomic_int32* waiters) {
    if (addr == NULL) {
        return;
    }

    if (waiters) {
        atomic_fetch_add(waiters, 1);
    }

    while (*addr) {
        if (kernel_initialized) {
            ktask_queue_flush();
        } else {
            irq_wait();
        }
    }

    if (waiters) {
        atomic_fetch_sub(waiters, 1);
    }
}

void spin_lock(spin_lock_t* lock) {
#ifdef ARCH_NO_SPINLOCK
    unused(lock);
    return;
#else
    bool warned = false;
    while (atomic_exchange(&lock->addr, 1)) {
        if (!warned) {
            warned = true;
            printf(WARN "Waiting for lock...\n");
        }
        spin_wait(&lock->addr, &lock->waiters);
    }
#endif
}

void spin_init(spin_lock_t* lock) {
    memset(lock, 0, sizeof(spin_lock_t));
}

void spin_unlock(spin_lock_t* lock) {
#ifdef ARCH_NO_SPINLOCK
    unused(lock);
    return;
#else
    if (lock->addr) {
        atomic_store(&lock->addr, 0);
        if (lock->waiters && kernel_initialized) {
            ktask_queue_flush();
        }
    }
#endif
}
