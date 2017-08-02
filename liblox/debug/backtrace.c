#include "backtrace.h"

void backtrace(trace_t* traces, uint size) {
    uint* ebp = (uint*) (&traces - 2);
    bool end = false;
    for (uint frame = 0; frame < size; frame++) {
        trace_t* current = &traces[frame];
        if (end) {
            current->call = NULL;
        }

        uintptr_t eip = ebp[1];
        if (eip == 0) {
            end = true;
            current->call = NULL;
            continue;
        }

        ebp = (uint*) ebp[0];
        current->call = (void*) eip;
    }
}
