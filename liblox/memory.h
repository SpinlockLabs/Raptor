#pragma once

#include <stddef.h>

int memcmp(const void* vl, const void* vr, size_t n);
void *memchr(const void *src, int c, size_t n);

void* malloc(size_t size);
void* zalloc(size_t size);
void free(void *ptr);
