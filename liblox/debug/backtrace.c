#include "backtrace.h"

void backtrace(uintptr_t* ebp, trace_t* traces, uint size) {
    if (ebp == NULL) {
        ebp = (uintptr_t*) (&traces - 3);
    }

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

        ebp = (uintptr_t*) ebp[0];
        current->call = (void*) eip;
    }
}
