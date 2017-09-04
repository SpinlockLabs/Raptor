#include <liblox/lox-internal.h>
#include <liblox/string.h>

extern int main(int argc, char** argv);

void __output_string(char* s) {
    size_t len = strlen(s);
    syscall(SYSCALL_CONSOLE_WRITE, s, len);
}

void __output_char(char c) {
    unused(c);
}

void __abort(char* msg) {
    unused(msg);
}

used void _start(void) {
    char* args[1] = {
            "exe"
    };
    main(1, args);
}

extern syscall_result_t __syscall(syscall_id_t id, uintptr_t* args);

void (*lox_output_string_provider)(char*) = __output_string;
void (*lox_output_char_provider)(char) = __output_char;
void (*lox_abort_provider)(char*) = __abort;
syscall_result_t (*lox_syscall_provider)(syscall_id_t, uintptr_t*) = __syscall;
