#include <stddef.h>

#include <liblox/string.h>

#include "idt.h"
#include "isr.h"

#define isr(i) idt_set_gate(i, (uint32_t)_isr##i, 0x08, 0x8E)

// Initializes the IDT.
void idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base = (uint32_t) &idt_entries;

    memset(&idt_entries, 0, sizeof (idt_entry_t) * 256);

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
    isr(127);

    idt_flush((uint32_t*) (uint32_t) &idt_ptr);
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low = (uint16_t) (base & 0xFFFF);
    idt_entries[num].base_high = (uint16_t) ((base >> 16) & 0xFFFF);

    idt_entries[num].sel = sel;
    idt_entries[num].zero = 0;
    idt_entries[num].flags = (uint8_t) (flags | 0x60);
}
