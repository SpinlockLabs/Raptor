#include "io.h"
#include "gdt.h"
#include "pm.h"

void enter_protected_mode(void) {
    asm volatile("cli");

    // Disable interrupts from PIC.
    /*outb(0xff, 0xa1);
    io_wait();
    outb(0xff, 0x21);
    io_wait();*/

    // Setup IDT
    static const gdt_ptr_t null_idt = {0, 0};
    asm volatile("lidtl %0" : : "m" (null_idt));

    // Setup GDT
    gdt_init();

    // Protected Mode Jump
    protected_mode_jump();
}
