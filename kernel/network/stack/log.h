#pragma once

#include <liblox/io.h>

#define dbg(msg, ...) printf(DEBUG "[Network Stack] " msg, ##__VA_ARGS__)
#define info(msg, ...) printf(INFO "[Network Stack] " msg, ##__VA_ARGS__)
#define warn(msg, ...) printf(WARN "[Network Stack] " msg, ##__VA_ARGS__)

#define ip_cp(ip) ((uint8_t*) (&(ip)))[0], \
     ((uint8_t*) (&(ip)))[1], \
     ((uint8_t*) (&(ip)))[2], \
     ((uint8_t*) (&(ip)))[3]

#define L_IP_FMT "%d.%d.%d.%d"
#define L_MAC_FMT "%2x:%2x:%2x:%2x:%2x:%2x"

#define mac_cp(mac) \
    mac[0], \
    mac[1], \
    mac[2], \
    mac[3], \
    mac[4], \
    mac[5]
