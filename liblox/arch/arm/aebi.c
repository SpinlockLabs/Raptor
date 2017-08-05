#include <stddef.h>

#include <liblox/alias.h>

extern void __memset(void* dest, size_t n, size_t c);
extern void __memmove(void* dest, const void* src, size_t n);
extern void __memcpy(void* restrict dest, const void* restrict src, size_t n);

void __aeabi_memclr(void* dest, size_t n) {
    __memset(dest, 0, n);
}

void __aeabi_memcpy(void* restrict dest, const void* restrict src, size_t n) {
    __memcpy(dest, src, n);
}

void __aeabi_memmove(void* dest, const void* src, size_t n) {
    __memmove(dest, src, n);
}

void __aeabi_memset(void* dest, size_t n, size_t c) {
    __memset(dest, n, c);
}

weak_alias(__aeabi_memclr, __aeabi_memclr4);
weak_alias(__aeabi_memclr, __aeabi_memclr8);

weak_alias(__aeabi_memset, __aeabi_memset4);
weak_alias(__aeabi_memset, __aeabi_memset8);

weak_alias(__aeabi_memmove, __aeabi_memmove4);
weak_alias(__aeabi_memmove, __aeabi_memmove8);

weak_alias(__aeabi_memcpy, __aeabi_memcpy4);
weak_alias(__aeabi_memcpy, __aeabi_memcpy8);
