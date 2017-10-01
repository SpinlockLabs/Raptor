#pragma once

#include <kernel/arch.h>

arch_specific void paging_init(void);

arch_specific page_directory_t* paging_clone_directory(page_directory_t*);
arch_specific page_table_t* paging_clone_table(page_table_t* src, uintptr_t* phys);

arch_specific void paging_switch_directory(page_directory_t*);
