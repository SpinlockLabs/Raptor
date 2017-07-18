#include "cmdline.h"

char *cmdline = NULL;

void init_cmdline(multiboot_t* mboot) {
    cmdline = (char*) mboot->cmdline;
}

char* arch_x86_get_cmdline(void) {
    return cmdline;
}

char* (*arch_get_cmdline)(void) = arch_x86_get_cmdline;
