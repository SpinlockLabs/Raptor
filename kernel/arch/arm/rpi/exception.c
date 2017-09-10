#include "exception.h"

#include <liblox/io.h>
#include <liblox/common.h>

used void exception_handler(uint32_t exception_id, AbortFrame* frame) {
    printf(ERROR "CPU Exception (eip = 0x%x)\n", frame->pc);
}
