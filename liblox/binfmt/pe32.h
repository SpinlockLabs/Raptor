/**
 * Portable Executable binfmt support.
 */
#pragma once

#include <stdint.h>

#include "../common.h"

#define PE32_MSDOS_STUB \
  { \
    0x4d, 0x5a, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, \
    0x04, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, \
    0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, \
    0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, \
    0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21, 0x54, 0x68, \
    0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, \
    0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f, \
    0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6e, \
    0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20, \
    0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a, \
    0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  \
  }

#define PE32_MSDOS_STUB_SIZE 0x80

#define PE32_SECTION_ALIGNMENT 0x200
#define PE32_FILE_ALIGNMENT PE32_SECTION_ALIGNMENT

typedef struct Pe32_Coff_Header {
    uint16_t machine;
    uint16_t section_count;
    uint32_t time;
    uint32_t symtab_offset;
    uint32_t symbol_count;
    uint16_t optional_header_size;
    uint16_t characteristics;
} packed Pe32_Coff_Header;

#define PE32_MACHINE_I386 0x14c
#define PE32_MACHINE_X86_64 0x8664

typedef struct Pe32_Data_Directory {
    uint32_t rva;
    uint32_t size;
} packed Pe32_Data_Directory;

typedef struct Pe32_Optional_Header {
    uint16_t magic;
    uint8_t major_linker_version;
    uint8_t minor_linker_version;
    uint32_t code_size;
    uint32_t data_size;
    uint32_t bss_size;
    uint32_t entry_addr;
    uint32_t code_base;

#ifdef BITS_32
    uint32_t data_base;
    uint32_t image_base;
#else
    uint64_t image_base;
#endif

    uint32_t section_alignment;
    uint32_t file_alignment;
    uint16_t major_os_version;
    uint16_t minor_os_version;
    uint16_t major_image_version;
    uint16_t minor_image_version;
    uint16_t major_subsystem_version;
    uint16_t minor_subsystem_version;
    uint32_t reserved;
    uint32_t image_size;
    uint32_t header_size;
    uint32_t checksum;
    uint16_t subsystem;
    uint16_t dll_characteristics;

#ifdef BITS_32
    uint32_t stack_reserve_size;
    uint32_t stack_commit_size;
    uint32_t heap_reserve_size;
    uint32_t heap_commit_size;
#else
    uint64_t stack_reserve_size;
    uint64_t stack_commit_size;
    uint64_t heap_reserve_size;
    uint64_t heap_commit_size;
#endif

    uint32_t loader_flags;
    uint32_t num_data_directories;

    Pe32_Data_Directory export_table;
    Pe32_Data_Directory import_table;
    Pe32_Data_Directory resource_table;
    Pe32_Data_Directory exception_table;
    Pe32_Data_Directory certificate_table;
    Pe32_Data_Directory base_relocation_table;
    Pe32_Data_Directory debug;
    Pe32_Data_Directory architecture;
    Pe32_Data_Directory global_ptr;
    Pe32_Data_Directory tls_table;
    Pe32_Data_Directory load_config_table;
    Pe32_Data_Directory bound_import;
    Pe32_Data_Directory iat;
    Pe32_Data_Directory delay_import_descriptor;
    Pe32_Data_Directory com_runtime_header;
    Pe32_Data_Directory reserved_entry;
} packed Pe32_Optional_Header;

#ifdef BITS_32
#define PE32_MAGIC 0x10b
#else
#define PE32_MAGIC 0x20b
#endif

#define PE32_NUM_DATA_DIRS 16

typedef struct Pe32_Section_Table {
    char name[8];
    uint32_t virtual_size;
    uint32_t virtual_addr;
    uint32_t raw_data_size;
    uint32_t raw_data_offset;
    uint32_t relocations_offset;
    uint32_t line_numbers_offset;
    uint16_t relocations_count;
    uint16_t line_numbers_count;
    uint32_t characteristics;
} packed Pe32_Section_Table;

#define PE32_SCN_CNT_CODE 0x00000020
#define PE32_SCN_CNT_INITIALIZED_DATA 0x00000040
#define PE32_SCN_MEM_DISCARDABLE 0x02000000
#define PE32_SCN_MEM_EXECUTE 0x20000000
#define PE32_SCN_MEM_READ 0x40000000
#define PE32_SCN_MEM_WRITE	0x80000000

#define PE32_SCN_ALIGN_1BYTES 0x00100000
#define PE32_SCN_ALIGN_2BYTES 0x00200000
#define PE32_SCN_ALIGN_4BYTES 0x00300000
#define PE32_SCN_ALIGN_8BYTES 0x00400000
#define PE32_SCN_ALIGN_16BYTES	0x00500000
#define PE32_SCN_ALIGN_32BYTES	0x00600000
#define PE32_SCN_ALIGN_64BYTES	0x00700000

#define PE32_SCN_ALIGN_SHIFT 20
#define PE32_SCN_ALIGN_MASK 7

typedef struct Pe32_Header {
    uint8_t msdos_stub[PE32_MSDOS_STUB_SIZE];
    char signature[4];
    Pe32_Coff_Header coff_header;
    Pe32_Optional_Header optional_header;
} packed Pe32_Header;

typedef struct Pe32_Fixup_Block {
    uint32_t page_rva;
    uint32_t block_size;
    uint16_t entries[0];
} packed Pe32_Fixup_Block;

#define PE32_FIXUP_ENTRY(type, offset) (((type) << 12) | (offset))

#define PE32_REL_BASED_ABSOLUTE 0
#define PE32_REL_BASED_HIGH 1
#define PE32_REL_BASED_LOW	2
#define PE32_REL_BASED_HIGHLOW	3
#define PE32_REL_BASED_HIGHADJ	4
#define PE32_REL_BASED_MIPS_JMPADDR 5
#define PE32_REL_BASED_SECTION	6
#define PE32_REL_BASED_REL	7
#define PE32_REL_BASED_IA64_IMM64 9
#define PE32_REL_BASED_DIR64 10
#define PE32_REL_BASED_HIGH3ADJ 11

typedef struct Pe32_Symbol {
    union {
        char short_name[8];
        uint32_t long_name[2];
    };

    uint32_t value;
    uint16_t section;
    uint16_t type;
    uint8_t storage_class;
    uint8_t aux_count;
} packed Pe32_Symbol;

#define PE32_SYM_CLASS_EXTERNAL 2
#define PE32_SYM_CLASS_STATIC 3
#define PE32_SYM_CLASS_FILE 0x67

#define PE32_DT_FUNCTION 0x20

typedef struct Pe32_Relocation {
    uint32_t offset;
    uint32_t symtab_index;
    uint16_t type;
} packed Pe32_Relocation;

#define PE32_REL_I386_DIR32 0x6
#define PE32_REL_I386_REL32 0x14
