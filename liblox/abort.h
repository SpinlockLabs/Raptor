#pragma once

#include "common.h"

/**
 * Abort the program.
 * In kernel mode, this calls a kernel panic.
 * @param msg abort message
 */
does_not_return void abort(char* msg);
