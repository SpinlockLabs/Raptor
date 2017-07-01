#pragma once

#include <stdint.h>

void outb(uint16_t, uint8_t);
uint8_t inb(uint16_t);
void outl(uint16_t, uint32_t);
uint32_t inl(uint16_t);
void outs(uint16_t, uint16_t);
uint16_t ins(uint16_t);
void io_wait(void);
