#include "env.h"

#define xstr(a) str(a)
#define str(a) #a

#ifdef RAPTOR
#define IS_RAPTOR true
#else
#define IS_RAPTOR false
#endif

#ifndef ARCH_NAME
#define ARCH_NAME generic
#endif

char* lox_env_arch = "" xstr(ARCH_NAME) "";
bool lox_env_is_raptor = IS_RAPTOR;
