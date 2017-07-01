#include <liblox/io.h>
#include <liblox/string.h>

#include <kernel/modload.h>

#include "multiboot.h"

void kernel_modules_load(void) {
    puts(DEBUG "Found ");

    char buf[32];
    itoa((int) mboot->mods_count, buf, 10);

    puts(buf);
    puts(" kernel modules.");
    puts("\n");
}
