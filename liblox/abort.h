#pragma once

#include "common.h"

/**
 * Abort the program.
 * In kernel mode, this calls a kernel panic.
 * @param msg abort message
 */
noreturn void abort(char* msg);
