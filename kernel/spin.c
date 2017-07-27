#include <liblox/atomic.h>

#include <liblox/io.h>

#include <kernel/spin.h>
#include <kernel/entry.h>

#include <kernel/cpu/task.h>
#include <kernel/interupt.h>

void spin_wait(volatile int* addr, volatile int* waiters) {
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

void spin_lock(spin_lock_t lock) {
    bool warned = false;
    while (atomic_exchange(lock, 1)) {
        if (!warned) {
            warned = true;
            printf(WARN "Waiting for lock...\n");
        }
        spin_wait(lock, lock + 1);
    }
}

void spin_init(spin_lock_t lock) {
    lock[0] = 0;
    lock[1] = 0;
}

void spin_unlock(spin_lock_t lock) {
    if (lock[0]) {
        atomic_store(lock, 0);
        if (lock[1] && kernel_initialized) {
            ktask_queue_flush();
        }
    }
}
