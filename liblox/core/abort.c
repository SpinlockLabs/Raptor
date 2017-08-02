#include "../abort.h"
#include "../lox-internal.h"
#include "../debug/backtrace.h"
#include "../io.h"

#define STACK_TRACE_SIZE 6

void abort(char* msg) {
    trace_t traces[STACK_TRACE_SIZE] = {0};

    backtrace(traces, STACK_TRACE_SIZE);

    for (uint i = 0; i < STACK_TRACE_SIZE; i++) {
        trace_t* trace = &traces[i];
        if (trace->call == NULL) {
            continue;
        }
        printf(TRACE "Call to 0x%x\n", trace->call);
    }

    if (lox_abort_provider != NULL) {
        lox_abort_provider(msg);
    }

    while (1) {}
}
