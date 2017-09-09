#include "irq.h"

#include <liblox/io.h>
#include <kernel/interupt.h>

#include "mmio.h"

#define ARM_OPCODE_BRANCH(distance)	(0xEA000000 | ((uint32_t) distance))
#define ARM_DISTANCE(from, to) ((void*) (((void*) to) - ((void*) from) - 2))
#define IRQ_MASK(irq) (1 << ((irq) & (BOARD_IRQS_PER_REG - 1)))

#define IRQ_PENDING(irq)	(  (irq) <BOARD_IRQ2_BASE	\
				 ? BOARD_IC_IRQ_PENDING_1		\
				 : ((irq) < BOARD_IRQBASIC_BASE	\
				   ? BOARD_IC_IRQ_PENDING_2	\
				   : BOARD_IC_IRQ_BASIC_PENDING))

#define IRQS_ENABLE(irq)	(  (irq) < BOARD_IRQ2_BASE	\
				 ? BOARD_IC_ENABLE_IRQS_1		\
				 : ((irq) < BOARD_IRQBASIC_BASE	\
				   ? BOARD_IC_ENABLE_IRQS_2	\
				   : BOARD_IC_ENABLE_BASIC_IRQS))

#define IRQS_DISABLE(irq) (  (irq) < BOARD_IRQ2_BASE	\
				 ? BOARD_IC_DISABLE_IRQS_1	\
				 : ((irq) < BOARD_IRQBASIC_BASE	\
				   ? BOARD_IC_DISABLE_IRQS_2	\
				   : BOARD_IC_DISABLE_BASIC_IRQS))

typedef struct exception_table {
    uint32_t undefined_instruction;
    uint32_t supervisor_call;
    uint32_t prefetch_abort;
    uint32_t data_abort;
    uint32_t unused;
    uint32_t irq;
    uint32_t fiq;
} exception_table_t;

void irq_wait(void) {
    printf(DEBUG "IRQ Wait\n");
}

extern void IRQStub(void);

void irq_init(void) {
    exception_table_t* table = (exception_table_t*) BOARD_EXCEPTION_TABLE_BASE;
    table->irq = ARM_OPCODE_BRANCH(ARM_DISTANCE(&table->irq, IRQStub));

    mmio_write(BOARD_IC_FIQ_CTL, 0);
    mmio_write(BOARD_IC_DISABLE_IRQS_1, (uint32_t) -1);
    mmio_write(BOARD_IC_DISABLE_IRQS_2, (uint32_t) -1);
    mmio_write(BOARD_IC_DISABLE_BASIC_IRQS, (uint32_t) -1);

    mmio_write(BOARD_IC_IRQ_BASIC_PENDING, mmio_read(BOARD_IC_IRQ_BASIC_PENDING));
    mmio_write(BOARD_IC_IRQ_PENDING_1, mmio_read(BOARD_IC_IRQ_PENDING_1));
    mmio_write(BOARD_IC_IRQ_PENDING_2, mmio_read(BOARD_IC_IRQ_PENDING_2));

    asm volatile("cpsie i;");
}

void irq_unmask(uint32_t irq) {
    uint32_t mask = (uint32_t) IRQ_MASK(irq);
    mmio_write(IRQS_ENABLE(irq), mask);
}

void irq_mask(uint32_t irq) {
    uint32_t mask = (uint32_t) IRQ_MASK(irq);
    mmio_write(IRQS_DISABLE(irq), mask);
}

void irq_handler(void) {
    printf(DEBUG "Handle IRQ\n");
    for (uint32_t irq = 0; irq < BOARD_IRQ_LINES; irq++) {
        uint32_t pending = IRQ_PENDING(irq);
        uint32_t irq_mask = (uint32_t) IRQ_MASK(irq);
        if (mmio_read(pending) & irq_mask) {
            printf(DEBUG "IRQ %d\n", irq);
        }
    }
}

typedef struct AbortFrame
{
    uint32_t	sp_irq;
    uint32_t	lr_irq;
    uint32_t	r0;
    uint32_t	r1;
    uint32_t	r2;
    uint32_t	r3;
    uint32_t	r4;
    uint32_t	r5;
    uint32_t	r6;
    uint32_t	r7;
    uint32_t	r8;
    uint32_t	r9;
    uint32_t	r10;
    uint32_t	r11;
    uint32_t	r12;
    uint32_t	sp;
    uint32_t	lr;
    uint32_t	spsr;
    uint32_t	pc;
} AbortFrame;

void exception_handler(uint32_t exception_id, AbortFrame* frame) {
    puts("Exception.\n");
}
