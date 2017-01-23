#pragma once

#define N 0x01 // Normal
#define S 0x02 // Special
#define F 0x03 // Function

static char key_types[] = {
    0, S, N, N, N, N, N, N,
    N, N, N, N, N, N, N, N,
    N, N, N, N, N, N, N, N,
    N, N, N, N, N, S, N, N,
    N, N, N, N, N, N, N, N,
    N, N, S, N, N, N, N, N,
    N, N, N, N, N, N, S, N,
    S, N, S, F, F, F, F, F,
    F, F, F, F, F, S, S, S,
    S, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, F,
    F, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, S,
    S, S, S, S, S, S, S, S
};

void keyboard_init(void);

