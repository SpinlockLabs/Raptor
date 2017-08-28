#include "../env.h"
#include "../entry.h"

#include <kernel/cpu/task.h>
#include <liblox/memory.h>
#include <liblox/string.h>

#include <emscripten.h>

int main(void) {
    kernel_main();
    return 0;
}

void* raptor_user_malloc(size_t size) {
  return EM_ASM_INT({
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

void async_do_run(void* arg) {
    ktask_queue_flush();
    raptor_user_process_stdin();
}

void cpu_run_idle(void) {
    int i = EM_ASM_INT({
      setInterval(
        function () {
            Module.ccall("ktask_queue_flush");
            Module.ccall("raptor_user_process_stdin");
        },
        1
      );
      return 0;
    }, 0);

    if (i) {
        ktask_queue_flush();
        raptor_user_process_stdin();
    }
}

char* _stdin_data = "";
bool _stdin_has_data = false;

used void rweb_write(char* msg) {
    _stdin_data = msg;
    _stdin_has_data = true;
}

void raptor_user_process_stdin(void) {
    if (_stdin_has_data) {
        _stdin_has_data = false;
        size_t len = strlen(_stdin_data);
        if (console_tty != NULL && console_tty->handle_read != NULL) {
            console_tty->handle_read(console_tty, _stdin_data, len);
        }
    }
}

void raptor_user_setup_devices(void) {}

void raptor_user_output_char(char c) {}

void raptor_user_output_string(char* str) {
    EM_ASM_({
        Module.print(Module.Pointer_stringify($0));
    }, str);
}

void raptor_user_abort(void) {}

void raptor_user_console_write(tty_t* tty, const uint8_t* buffer, size_t size) {
    uint8_t* data = zalloc(size + 1);
    memcpy(data, buffer, size);
    data[size] = '\0';
    EM_ASM_({
      Module.print(Module.Pointer_stringify($0));
    }, buffer);
    free(data);
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
