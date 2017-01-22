#pragma once
#include "isr.h"

#define IRQ_CHAIN_SIZE  16
#define IRQ_CHAIN_DEPTH 4

typedef int (*irq_handler_chain_t) (struct regs *);
static irq_handler_chain_t irq_routines[IRQ_CHAIN_SIZE * IRQ_CHAIN_DEPTH] = { NULL  };

void int_disable(void);
void int_resume(void);
void int_enable(void);
void irq_add_handler(size_t, irq_handler_chain_t);
void irq_remove_handler(size_t);
static void irq_remap();
void irq_handler(regs_t *);

extern void _irq0();
extern void _irq1();
extern void _irq2();
extern void _irq3();
extern void _irq4();
extern void _irq5();
extern void _irq6();
extern void _irq7();
extern void _irq8();
extern void _irq9();
extern void _irq10();
extern void _irq11();
extern void _irq12();
extern void _irq13();
extern void _irq14();

