/* kexec implementation from xinu. */

#include <liblox/string.h>

#include <kernel/kexec.h>

static const ulong copy_kernel[] = {
    0xe3a04902,
    0xe4903004,
    0xe4843004,
    0xe2511001,
    0x1afffffb,
    0xe3a0f902,
};

void kexec(const void* kernel, size_t size) {
    void* ckaddr = (void*) (0x8000 - sizeof(copy_kernel));
    memcpy(ckaddr, copy_kernel, sizeof(copy_kernel));
    extern void* atags;
    ((void (*)(const void*, ulong, void*))(ckaddr))(
        kernel,
        (size + 3),
        atags
    );
}
