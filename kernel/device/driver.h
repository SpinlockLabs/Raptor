#pragma once

typedef void (*driver_setup_t)(void);

void driver_register(driver_setup_t setup);
