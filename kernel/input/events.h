#pragma once

#include "input.h"

/**
 * Key down event.
 */
typedef struct input_event_key_state {
    /**
     * Input event.
     */
    input_event_t event;

    /**
     * Key code.
     */
    uint32_t key;

    union {
        struct {
            bool shift : 1;
            bool ctrl : 1;
        };

        uint32_t flags;
    };
} input_event_key_state_t;
