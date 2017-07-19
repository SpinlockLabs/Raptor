#pragma once

#include <stddef.h>
#include <stdint.h>

#include "memory.h"

typedef struct {
    unsigned char* data;
    size_t size;
} bitset_t;

void bitset_init(bitset_t* set, size_t size);
void bitset_free(bitset_t* set);
void bitset_set(bitset_t* set, size_t bit);
void bitset_clear(bitset_t* set, size_t bit);
int bitset_test(bitset_t* set, size_t bit);
int bitset_ffub(bitset_t* set);
