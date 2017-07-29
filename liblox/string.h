#include <stdbool.h>
#include <stddef.h>

size_t strlen(const char*);

void reverse(char*, int);

int atoi(const char* s);

char* itoa(int, char*, int);

int strcmp(const char*, const char*);

void* memset(void*, int, size_t);

void* memcpy(void*, const void*, size_t);

char* strdup(const char*);

size_t strspn(const char* s, const char* c);

size_t strcspn(const char* s, const char* c);

char* strchrnul(const char* s, int c);

char* strtok(char* restrict s, const char* restrict sep, char** restrict p);

char* strchr(const char* s, int c);

char* strstr(const char* h, const char* n);

long strtol(const char* nptr, char** endptr, register int base);

char* strncpy(char* restrict d, const char* restrict s, size_t n);

void* memchr(const void* src, int c, size_t n);

void* memmem(const void* h0, size_t k, const void* n0, size_t l);

size_t strnlen(const char* s, size_t n);

int memcmp(const void* vl, const void* vr, size_t n);
