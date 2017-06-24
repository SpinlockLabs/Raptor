#include "idt.h"
#include <stddef.h>
#include <string.h>

// Initializes the IDT.
void idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base = (uint32_t) &idt_entries;

    memset(&idt_entries, 0, sizeof (idt_entry_t) * 256);

    idt_flush((uint32_t *) (uint32_t) &idt_ptr);
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low = (uint16_t) (base & 0xFFFF);
    idt_entries[num].base_high = (uint16_t) ((base >> 16) & 0xFFFF);

    idt_entries[num].sel = sel;
    idt_entries[num].zero = 0;
    // Uncomment when we get user-mode.
    // Sets the interrupt gate privilege level to 3.
    idt_entries[num].flags = flags /* | 0x60 */;
}
