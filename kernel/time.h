#pragma once

#include <liblox/common.h>
#include <stdint.h>

/**
 * Time instance.
 */
typedef struct rtime {
    uint16_t second;
    uint16_t minute;
    uint16_t hour;
    uint16_t day;
    uint16_t month;
    uint16_t year;
} packed rtime_t;

/**
 * Retrieves the current time.
 * @param time time instance
 */
extern arch_specific void time_get(rtime_t* time);
