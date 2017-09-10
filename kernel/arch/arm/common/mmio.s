.globl mmio_write
mmio_write:
    str r1,[r0]
    bx lr

.globl mmio_read
mmio_read:
    ldr r0,[r0]
    bx lr
