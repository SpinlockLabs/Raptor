#pragma once

#include <liblox/io.h>

#define dbg(msg, ...) printf(DEBUG "[Network Stack] " msg, ##__VA_ARGS__)
#define info(msg, ...) printf(INFO "[Network Stack] " msg, ##__VA_ARGS__)

#define ip_cp(ip) ((uint8_t*) (&(ip)))[0], ((uint8_t*) (&(ip)))[1], ((uint8_t*) (&(ip)))[2], ((uint8_t*) (&(ip)))[3]
