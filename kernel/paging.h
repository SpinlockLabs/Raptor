#pragma once

#include <kernel/arch.h>

void paging_init(void);

page_directory_t* paging_clone_directory(page_directory_t*);
page_table_t* paging_clone_table(page_table_t* src, uintptr_t* phys);

void paging_switch_directory(page_directory_t*);
