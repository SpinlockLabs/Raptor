#pragma once

#include <liblox/io.h>

#define dbg(msg, ...) printf(DEBUG "[Network Stack] " msg, ##__VA_ARGS__)
#define info(msg, ...) printf(INFO "[Network Stack] " msg, ##__VA_ARGS__)
