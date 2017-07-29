#include <liblox/string.h>

#include <kernel/time.h>

void time_get(time_t* time) {
    memset(time, 0, sizeof(time_t));
}
