.set MB_MAGIC, 0x1BADB002
.set MB_FLAG_PAGE_ALIGN, 1 << 0
.set MB_FLAG_MEMORY_INFO, 1 << 1
.set MB_FLAG_GRAPHICS, 1 << 2
.set MB_FLAGS, MB_FLAG_PAGE_ALIGN | MB_FLAG_MEMORY_INFO
.set MB_CHECKSUM, -(MB_MAGIC + MB_FLAGS)

.section .multiboot
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM
.long 0x00000000 /* header_addr */
.long 0x00000000 /* load_addr */
.long 0x00000000 /* load_end_addr */
.long 0x00000000 /* bss_end_addr */
.long 0x00000000 /* entry_addr */

/* Request linear graphics mode */
.long 0x00000000
.long 0
.long 0
.long 32

.section .stack, "aw", @nobits
stack_bottom:
.skip 32768 # 32 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    # Setup the stack.
    mov $stack_top, %esp

    and $-16, %esp

    pushl %eax /* Multiboot header magic */
    pushl %ebx /* Multiboot header pointer */

    # Call the kernel entrypoint.
    call kernel_main

    # Disable interrupts, if we get to this,
    # the system has likely gone into a bad state.
    cli

    # Go into an infinite loop with hlt.
1:  hlt
    jmp 1b

# Apply the new global descriptor table.
.global gdt_flush
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %ss

    ljmp $0x08, $.flush
.flush:
    ret

# Apply the new interrupt descriptor table.
.global idt_flush
.type idt_flush, @function
idt_flush:
    mov 4(%esp), %eax
    lidt (%eax)
    ret

.size _start, . - _start
