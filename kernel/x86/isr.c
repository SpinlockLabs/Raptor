#include "isr.h"
#include "tty.h"

#define isr(i) idt_set_gate(i, (uint32_t)_isr##i, 0x08, 0x8E)

void isr_init(void) {
    // Code from toaruos, I think it's the syscall interrupt...
    //isr[32].index = SYSCALL_VECTOR;
    //isr[32].stub = symbol_find("_isr127");

    isr(0);
    isr(1);
    isr(2);
    isr(3);
    isr(4);
    isr(5);
    isr(6);
    isr(7);
    isr(8);
    isr(9);
    isr(10);
    isr(11);
    isr(12);
    isr(13);
    isr(14);
    isr(15);
    isr(16);
    isr(17);
    isr(18);
    isr(19);
    isr(20);
    isr(21);
    isr(22);
    isr(23);
    isr(24);
    isr(25);
    isr(26);
    isr(27);
    isr(28);
    isr(29);
    isr(30);
    isr(31);
}

void fault_handler(regs_t *r) {
    terminal_writestring("Unhandled exception\n");
}

