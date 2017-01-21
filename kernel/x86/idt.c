#include "idt.h"
#include <stddef.h>

void* memset(void* bufptr, int value, size_t size) {
    unsigned char* buf = (unsigned char*) bufptr;
    for (size_t i = 0; i < size; i++)
        buf[i] = (unsigned char) value;
    return bufptr;
}

// Function in assembly to run lidt instruction with IDT location.
extern void idt_load(void);

// Initializes the IDT.
void idt_init(void) {
    idtp.limit = (sizeof(idt_entry) * 256) - 1;
    idtp.base = &idt;

    memset(&idt, 0, sizeof (idt_entry) * 256);

    idt_load();
}
