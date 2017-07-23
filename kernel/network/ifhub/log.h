#pragma once

#include <liblox/io.h>

#define dbg(msg, ...) printf(DEBUG "[Network Hub] " msg, ##__VA_ARGS__)
#define info(msg, ...) printf(INFO "[Network Hub] " msg, ##__VA_ARGS__)
#define warn(msg, ...) printf(WARN "[Network Hub] " msg, ##__VA_ARGS__)
