#include "idt.h"
#include <stddef.h>

void* memset(void* bufptr, int value, size_t size) {
    unsigned char* buf = (unsigned char*) bufptr;
    for (size_t i = 0; i < size; i++)
        buf[i] = (unsigned char) value;
    return bufptr;
}

// Initializes the IDT.
void idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base = &idt_entries;

    memset(&idt_entries, 0, sizeof (idt_entry_t) * 256);

    idt_flush();
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;

    idt_entries[num].sel = sel;
    idt_entries[num].zero = 0;
    // Uncomment when we get user-mode.
    // Sets the interrupt gate privilege level to 3.
    idt_entries[num].flags = flags /* | 0x60 */;
}
