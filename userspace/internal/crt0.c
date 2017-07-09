#include <liblox/syscall.h>
#include <liblox/lox-internal.h>

extern int main(int argc, char** argv);

void __output_string(char* s) {
    unused(s);
}

void __output_char(char c) {
    unused(c);
}

void _start(void) {
    char* args[0];
    int ret = main(0, args);

    syscall(SYSCALL_EXIT, ret);
}

void (*lox_output_string_provider)(char*) = __output_string;
void (*lox_output_char_provider)(char) = __output_char;
