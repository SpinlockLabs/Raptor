#include <stdbool.h>
#include <stddef.h>

size_t strlen(const char*);
void reverse(char*, int);
char* itoa(int, char*, int);
int strcmp(const char*, const char*);
void* memset(void*, int, size_t);
void* memcpy(void*, const void*, size_t);
char* strdup(const char*);
size_t strspn(const char* s, const char* c);
size_t strcspn(const char* s, const char* c);
char *strchrnul(const char *s, int c);
char* strtok(char* restrict s, const char* restrict sep, char** restrict p);
