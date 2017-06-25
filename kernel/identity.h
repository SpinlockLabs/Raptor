#pragma once

#include <stdint.h>

// Defines core system identity interfaces.
typedef struct system_identity_t {
    char arch_name[12]; // A short architecture name.
    char processor_name[30];// A longer processor identifier.
    uint8_t bits; // System bits.
} system_identity_t;
