#include "irq.h"
#include "io.h"
#include "tty.h"
#include <stdint.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_CMD PIC1
#define PIC2_CMD PIC2
#define PIC1_DATA (PIC1+1)
#define PIC2_DATA (PIC2+1)
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

#define irq(i) idt_set_gate(i + 32, (uint32_t)_irq##i, 0x08, 0x8E)
#define sti() asm volatile("sti");
#define cli() asm volatile("cli");

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
    // @TODO
    sti();
}

void irq_rem_handler(size_t irq) {
    cli();
    // @TODO
    sti();
}

static void irq_remap() {
    // @TODO: doesn't look like toaruos does this, check if necessary
    // https://github.com/klange/toaruos/blob/f17cf012ca01b416ff12b3520f6a8fca6e5a05ef/kernel/cpu/irq.c
    uint8_t a1, a2;
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

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
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    // Request 8086 mode on both PICs.
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
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
}

void irq_init(void) {
    irq_remap();
    irq_setup_gates();
}

void irq_handler(regs_t *r) {
    int_disable();


    // Reset slave PIC if eighth interrupt or higher.
    int i = r->int_no - 32;
    if (i >= 8) {
        outb(PIC2_CMD, 0x20);
    }
    // Reset master PIC.
    outb(PIC1_CMD, 0x20);

    if (i == 0) {
        // Ignore tick for now.
    } else {
        terminal_writestring("Unhandled interrupt ");
        char* is;
        itoa(i, is, 10);
        terminal_writestring(is);
        terminal_writestring("\n");
    }

    int_resume();
}

