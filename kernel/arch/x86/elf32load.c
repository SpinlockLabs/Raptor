#include "elf32load.h"

#include <liblox/string.h>

#include <liblox/binfmt/elf32.h>
#include <kernel/process/process.h>

#include "paging.h"
#include "irq.h"
#include "userspace.h"

#define PUSH(stack, type, item) stack -= sizeof(type); \
							*((type *) stack) = item

void enter_user_jmp(uintptr_t location, int argc, char ** argv, uintptr_t stack) {
    int_disable();
    set_kernel_stack(process_get_current()->image.stack);

    PUSH(stack, uintptr_t, (uintptr_t) argv);
    PUSH(stack, int, argc);
    enter_userspace((void*) location, stack);
}

bool elf32_execute(
    uint8_t* bytes,
    size_t size,
    int argc,
    char** argv,
    int envc,
    char** env) {
    process_t* process = process_get_current();

    Elf32_Header* header = (Elf32_Header*) bytes;
    if (header->e_ident[0] != ELFMAG0 ||
        header->e_ident[1] != ELFMAG1 ||
        header->e_ident[2] != ELFMAG2 ||
        header->e_ident[3] != ELFMAG3) {
        return false;
    }

    uintptr_t entry = (uintptr_t) header->e_entry;
    uintptr_t base_addr = 0xFFFFFFFF;
    uintptr_t end_addr  = 0x0;

    for (uintptr_t x = 0; x < (uint32_t) header->e_phentsize * header->e_phnum; x += header->e_phentsize) {
        Elf32_Phdr* phdr = (Elf32_Phdr*) (bytes + x);
        if (phdr->p_type == PT_LOAD) {
            if (phdr->p_vaddr < base_addr) {
                base_addr = phdr->p_vaddr;
            }

            if (phdr->p_memsz + phdr->p_vaddr > end_addr) {
                end_addr = phdr->p_memsz + phdr->p_vaddr;
            }
        }
    }

    process->image.entry = entry;
    process->image.size = end_addr - base_addr;


    paging_release_directory_for_exec(paging_get_directory());
    paging_invalidate_tables();

    for (uintptr_t x = 0; x < (uint32_t) header->e_phentsize * header->e_phnum; x += header->e_phentsize) {
        Elf32_Phdr* phdr = (Elf32_Phdr*) (bytes + header->e_phoff + x);
        if (phdr->p_type == PT_LOAD) {
            for (uintptr_t i = phdr->p_vaddr; i < phdr->p_vaddr + phdr->p_memsz; i += 0x1000) {
                /* This doesn't care if we already allocated this page */
                paging_allocate_frame(paging_get_page(i, 1, paging_get_directory()), 0, 1);
                paging_invalidate_tables_at(i);
            }
            memcpy((void*) phdr->p_vaddr, bytes + phdr->p_offset, phdr->p_filesz);
            size_t r = phdr->p_filesz;
            while (r < phdr->p_memsz) {
                *(char *)(phdr->p_vaddr + r) = 0;
                r++;
            }
        }
    }

    for (uintptr_t stack_pointer = USER_STACK_BOTTOM; stack_pointer < USER_STACK_TOP; stack_pointer += 0x1000) {
        paging_allocate_frame(paging_get_page(stack_pointer, 1, paging_get_directory()), 0, 1);
        paging_invalidate_tables_at(stack_pointer);
    }

    Elf32_auxv auxv[] = {
        {256, 0xDEADBEEF},
        {0, 0}
    };
    int auxvc = 0;
    for (auxvc = 0; auxv[auxvc].id != 0; ++auxvc);
    auxvc++;

    uintptr_t heap = process->image.entry + process->image.size;
    while (heap & 0xFFF) heap++;

    paging_allocate_frame(paging_get_page(heap, 1, paging_get_directory()), 0, 1);
    paging_invalidate_tables_at(heap);

    char ** argv_ = (char **)heap;
    heap += sizeof(char *) * (argc + 1);
    char ** env_ = (char **)heap;
    heap += sizeof(char *) * (0 + 1);
    void * auxv_ptr = (void *)heap;
    heap += sizeof(Elf32_auxv) * (auxvc);

    for (int i = 0; i < argc; ++i) {
        size_t argsz = strlen(argv[i]) * sizeof(char) + 1;
        for (uintptr_t x = heap; x < heap + argsz + 0x1000; x += 0x1000) {
            paging_allocate_frame(paging_get_page(x, 1, paging_get_directory()), 0, 1);
        }
        paging_invalidate_tables_at(heap);
        argv_[i] = (char *)heap;
        memcpy((void *)heap, argv[i], argsz);
        heap += argsz;
    }

    argv_[argc] = 0;

    for (int i = 0; i < envc; ++i) {
        size_t esize = strlen(env[i]) * sizeof(char) + 1;
        for (uintptr_t x = heap; x < heap + esize + 0x1000; x += 0x1000) {
            paging_allocate_frame(paging_get_page(x, 1, paging_get_directory()), 0, 1);
        }
        paging_invalidate_tables_at(heap);
        env_[i] = (char*) heap;
        memcpy((void *)heap, env[i], esize);
        heap += esize;
    }
    env_[envc] = 0;

    memcpy(auxv_ptr, auxv, sizeof(Elf32_auxv) * (auxvc));
    process->image.heap = heap;
    process->image.heap_actual = heap + (0x1000 - heap % 0x1000);
    paging_allocate_frame(paging_get_page(process->image.heap_actual, 1, paging_get_directory()), 0, 1);
    paging_invalidate_tables_at(process->image.heap_actual);
    process->image.user_stack = USER_STACK_TOP;
    enter_user_jmp(entry, argc, argv_, USER_STACK_TOP);
    return true;
}
