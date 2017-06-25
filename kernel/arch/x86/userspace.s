.section .text
.align 4

.global enter_userspace
.type enter_userspace, @function
enter_userspace:
    // Segment selector
    mov $0x23, %ax

    // Save segment registers
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs

    // Store stack address in eax
    mov %esp, %eax

    // Push stack address
    pushl $0x23

    // Push the stack address
    pushl %eax

    // Push flags
    pushf

    // Request ring 3
    pushl $0x1B
    push $1f
    iret
1:
    hlt
