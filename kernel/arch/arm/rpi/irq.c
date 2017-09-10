#include "irq.h"
#include "../common/mmio.h"

#include <kernel/interupt.h>

#include <liblox/io.h>
#include <liblox/common.h>

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
extern void DataAbortStub(void);
extern void PrefetchAbortStub(void);
extern void DataAbortStub(void);
extern void UndefinedInstructionStub(void);

void irq_init(void) {
    exception_table_t* table = (exception_table_t*) BOARD_EXCEPTION_TABLE_BASE;

    #define SET_TABLE_STUB(id, func) \
        id = ARM_OPCODE_BRANCH(ARM_DISTANCE(&(id), #func))

    SET_TABLE_STUB(table->irq, IRQStub);
    SET_TABLE_STUB(table->data_abort, DataAbortStub);
    SET_TABLE_STUB(table->data_abort, PrefetchAbortStub);
    SET_TABLE_STUB(table->undefined_instruction, UndefinedInstructionStub);

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

typedef struct irq_handler_info {
    irq_handler_t* handler;
    void* data;
} irq_handler_info_t;

static irq_handler_info_t irq_handlers[BOARD_IRQ_LINES];

used void irq_handler(void) {
    for (uint32_t irq = 0; irq < BOARD_IRQ_LINES; irq++) {
        uint32_t pending = IRQ_PENDING(irq);
        uint32_t irq_mask = (uint32_t) IRQ_MASK(irq);
        if (mmio_read(pending) & irq_mask) {
            printf(DEBUG "[IRQ] %d\n", irq);
            if (irq_handlers[irq].handler != NULL) {
                irq_handlers[irq].handler(irq_handlers[irq].data);
            }
        }
    }
}

void irq_set_handler(uint32_t irq, irq_handler_t handler, void* data) {
    irq_handlers[irq].handler = handler;
    irq_handlers[irq].data = data;

    irq_unmask(irq);
}
