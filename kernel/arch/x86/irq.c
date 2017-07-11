#include <liblox/common.h>

#include "idt.h"
#include "io.h"
#include "irq.h"

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_CMD PIC1
#define PIC2_CMD PIC2
#define PIC1_DATA (PIC1+1)
#define PIC2_DATA (PIC2+1)
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

#define ICW1_ICW4 0x01
#define ICW1_INTERVAL4 0x04
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02

#define irq(i) idt_set_gate((i) + 32, (uint32_t)_irq##i, 0x08, 0x8E)
#define sti() asm volatile("sti");
#define cli() asm volatile("cli");

static irq_handler_chain_t irq_routines[IRQ_CHAIN_SIZE * IRQ_CHAIN_DEPTH] = { NULL };

static volatile int sync_depth = 0;

void int_disable(void) {
    uint32_t flags;
    asm volatile("pushf\n\t"
                 "pop %%eax\n\t"
                 "movl %%eax, %0\n\t"
                 : "=r"(flags)
                 :
                 : "%eax");

    cli();

    // First call depth if interrupts were enabled.
    if (flags & (1 << 9)) {
        sync_depth = 1;
    } else {
        // Increment call depth.
        sync_depth++;
    }
}

void int_resume(void) {
    if (sync_depth == 0 || sync_depth == 1) {
        sti();
    } else {
        sync_depth--;
    }
}

void int_enable(void) {
    sync_depth = 0;
    sti();
}

void irq_add_handler(size_t irq, irq_handler_chain_t handler) {
    cli();
    for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
        if (irq_routines[i * IRQ_CHAIN_SIZE + irq]) {
            continue;
        }
        irq_routines[i * IRQ_CHAIN_SIZE + irq] = handler;
        break;
    }
    sti();
}

void irq_remove_handler(size_t irq) {
    cli();
    for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
        irq_routines[i * IRQ_CHAIN_SIZE + irq] = NULL;
    }
    sti();
}

static void irq_remap() {
    // Cascade init.
    outb(PIC1_CMD, ICW1_INIT + ICW1_ICW4);
    io_wait();
    outb(PIC2_CMD, ICW1_INIT + ICW1_ICW4);
    io_wait();

    // Remap each PIC.
    outb(PIC1_DATA, PIC1_OFFSET);
    io_wait();
    outb(PIC2_DATA, PIC2_OFFSET);
    io_wait();

    // Cascade identity with slave PIC at IRQ2.
    outb(PIC1_DATA, ICW1_INTERVAL4);
    io_wait();
    outb(PIC2_DATA, ICW4_AUTO);
    io_wait();

    // Request 8086 mode on both PICs.
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
}

static void irq_setup_gates(void) {
    irq(0);
    irq(1);
    irq(2);
    irq(3);
    irq(4);
    irq(5);
    irq(6);
    irq(7);
    irq(8);
    irq(9);
    irq(10);
    irq(11);
    irq(12);
    irq(13);
    irq(14);
    irq(15);
}

void irq_init(void) {
    irq_remap();
    irq_setup_gates();
}

/**
 * Reset master PIC, reset slave if IRQ >= 8.
 */
void irq_ack(size_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, 0x20);
    }
    outb(PIC1_CMD, 0x20);
}

used void irq_handler(regs_t *r) {
    int_disable();

    if (r->int_no <= 47 && r->int_no >= 32) {
        for (size_t i = 0; i < IRQ_CHAIN_DEPTH; i++) {
            int index = i * IRQ_CHAIN_SIZE + (r->int_no - 32);
            irq_handler_chain_t handler = irq_routines[index];
            if (handler && handler(r)) {
                goto done;
            }
        }
    }
done:
    irq_ack(r->int_no - 32);
    int_resume();
}
