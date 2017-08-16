#include "../env.h"
#include "../entry.h"

#include <emscripten.h>

int main(void) {
  kernel_main();
}

void* raptor_user_malloc(size_t size) {
  return (void*) EM_ASM_INT({
    return Module.Runtime.dynamicAlloc($0);
  }, size);
}

void* raptor_user_valloc(size_t size) {
  return NULL;
}

void raptor_user_free(void* ptr) {
}

void* raptor_user_realloc(void* ptr, size_t size) {
  return NULL;
}

void raptor_user_process_stdin(void) {}

void raptor_user_setup_devices(void) {}

void raptor_user_output_char(char c) {}

void raptor_user_output_string(char* str) {}

void raptor_user_abort(void) {}

void raptor_user_console_write(tty_t* tty, const uint8_t* buffer, size_t size) {
}

char* raptor_user_get_cmdline(void) {
  return "";
}

void raptor_user_exit(void) {
}

static ulong ticks = 0;

ulong raptor_user_ticks(void) {
  return ticks++;
}

void raptor_user_get_time(rtime_t* rt) {
}
