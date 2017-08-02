#pragma once

#define INFO "[INFO ] "
#define DEBUG "[DEBUG] "
#define WARN "[WARN ] "
#define ERROR "[ERROR] "
#define TRACE "[TRACE] "

#include "va_list.h"

void puts(char*);
void putc(char);
void printf(char* fmt, ...);
