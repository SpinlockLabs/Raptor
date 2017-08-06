#include "driver.h"

#include <liblox/common.h>

#include <kernel/dispatch/events.h>

static void do_driver_setup(void* event, void* extra) {
    unused(event);

    driver_setup_t setup = extra;
    setup();
}

void driver_register(driver_setup_t setup) {
    event_add_handler(EVENT_DRIVER_SETUP, do_driver_setup, setup);
}
