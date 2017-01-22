#include <stdint.h>

typedef struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t zero;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;

void idt_init(void);
void idt_set_gate(uint8_t, uint32_t, uint16_t, uint8_t);

extern void idt_flush(void);

