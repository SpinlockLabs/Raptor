#include "debug.h"
#include "heap.h"
#include "paging.h"
#include "elf32load.h"

#include <kernel/tty.h>

#include <kernel/debug/console.h>

#include <kernel/arch/x86/devices/pci/pci.h>
#include <kernel/fs/vfs.h>
#include <liblox/memory.h>

static void debug_kpused(tty_t* tty, const char* input) {
    unused(input);

    size_t size = kpused();
    int kb = size / 1024;

    tty_printf(tty, "Used: %d bytes, %d kb\n", size, kb);
}

static void debug_page_stats(tty_t* tty, const char* input) {
    unused(input);

    uintptr_t memory_total = paging_memory_total();
    uintptr_t memory_used = paging_memory_used();

    tty_printf(tty,
               "Total Memory: %d bytes, %d kb\n",
               memory_total,
               (uintptr_t) (memory_total / 1024)
    );

    tty_printf(tty,
               "Used Memory: %d bytes, %d kb\n",
               memory_used,
               (uintptr_t) (memory_used / 1024)
    );
}

static void pci_show_simple(uint32_t loc, uint16_t vid, uint16_t did, void* extra) {
    tty_t* tty = extra;

    const char* vendor = pci_vendor_lookup(vid);
    const char* dev = pci_device_lookup(vid, did);

    if (vendor == NULL) {
        vendor = "Unknown";
    }

    if (dev == NULL) {
        dev = "Unknown";
    }

    tty_printf(tty,
               "(0x%x) [%s] (0x%x) by [%s] (0x%x)\n",
               loc, dev, did, vendor, vid);
}

static void debug_pci_list(tty_t* tty, const char* input) {
    unused(input);

    pci_scan(pci_show_simple, -1, tty);
}

static void debug_page_dump(tty_t* tty, const char* input) {
    unused(input);

    page_directory_t* current = paging_get_directory();

    for (uintptr_t i = 0; i < 1024; i++) {
        if (!current->tables[i] ||
            (uintptr_t) current->tables[i] == (uintptr_t) 0xFFFFFFFF) {
            continue;
        }

        for (uint16_t j = 0; j < 1024; j++) {
            page_t* p = &current->tables[i]->pages[j];
            if (p->frame) {
                tty_printf(
                    tty,
                    "page 0x%x 0x%x %s%s%s%s%s\n",
                    (i * 1024 + j) * 0x1000,
                    p->frame * 0x1000,
                    p->present ? "[present]" : "",
                    p->rw ? "[rw]" : "",
                    p->user ? "[user]" : "[kernel]",
                    p->writethrough ? "[writethrough]" : "",
                    p->nocache ? "[nocache]" : ""
                );
            }
        }
    }
}

static void debug_start_true(tty_t* tty, const char* input) {
    unused(tty);
    unused(input);

    fs_node_t* node = fs_resolve("/bin/true");
    if (node == NULL) {
        tty_printf(tty, "Failed to resolve '/bin/true' executable.\n");
        return;
    }

    fs_stat_t stat;
    if (fs_stat(node, &stat) != FS_ERROR_OK) {
        tty_printf(tty, "Failed to stat '/bin/true' executable.\n");
        return;
    }
    uint8_t* buff = zalloc(stat.size);
    if (fs_read(node, 0, buff, stat.size) != FS_ERROR_OK) {
        tty_printf(tty, "Failed to read '/bin/true' executable.\n");
        return;
    }

    char* argv[] = {""};

    extern void sysexec(
        char* name,
        uint8_t* bytes,
        size_t size,
        int argc,
        char**);

    sysexec("true", buff, stat.size, 0, argv);
}

void debug_x86_init(void) {
    debug_register_command("kpused", debug_kpused);
    debug_register_command("pci-list", debug_pci_list);
    debug_register_command("page-stats", debug_page_stats);
    debug_register_command("page-dump", debug_page_dump);
    debug_register_command("start-true", debug_start_true);
}
