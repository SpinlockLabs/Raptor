#include "cmdline.h"

char *cmdline = "";

void init_cmdline(multiboot_t* mboot) {
    cmdline = (char*) mboot->cmdline;
}

char* cmdline_get(void) {
    return cmdline;
}
