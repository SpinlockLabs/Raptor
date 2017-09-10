#pragma once

#include <stdint.h>

typedef struct AbortFrame {
    uint32_t sp_irq;
    uint32_t lr_irq;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t sp;
    uint32_t lr;
    uint32_t spsr;
    uint32_t pc;
} AbortFrame;

void exception_handler(uint32_t exception_id, AbortFrame* frame);
