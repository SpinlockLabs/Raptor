#pragma once

#include "va_list.h"

extern size_t vasprintf(char * buf, const char *fmt, va_list args);
extern int sprintf(char *buf, const char *fmt, ...);
