#include <liblox/string.h>

#include <kernel/time.h>

void time_get(rtime_t* time) {
    memset(time, 0, sizeof(time_t));
}
