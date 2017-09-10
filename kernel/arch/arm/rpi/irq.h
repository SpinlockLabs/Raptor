#pragma once

#include <stdint.h>
#include <kernel/dispatch/pipe.h>

#include "board.h"

#define BOARD_IRQS_PER_REG 32
#define BOARD_IRQ1_BASE 0
#define BOARD_IRQ2_BASE (BOARD_IRQ1_BASE + BOARD_IRQS_PER_REG)
#define BOARD_IRQBASIC_BASE (BOARD_IRQ2_BASE + BOARD_IRQS_PER_REG)
#define BOARD_IRQ_LINES (BOARD_IRQS_PER_REG * 2 + 8)

#define BOARD_IRQ_TIMER0 (BOARD_IRQ1_BASE + 0)
#define BOARD_IRQ_TIMER1 (BOARD_IRQ1_BASE + 1)
#define BOARD_IRQ_TIMER2 (BOARD_IRQ1_BASE + 2)
#define BOARD_IRQ_TIMER3 (BOARD_IRQ1_BASE + 3)

void irq_init(void);
void irq_unmask(uint32_t irq);
void irq_mask(uint32_t irq);

typedef void irq_handler_t(void* data);

void irq_set_handler(uint32_t irq, irq_handler_t handler, void* data);
