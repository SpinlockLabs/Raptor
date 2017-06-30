/* Bootloader looks at this image and starts at the symbol marked as the entrypoint. */
OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    /* Begin putting sections at 1 MiB, a conventional place for kernels to be loaded by the bootloader. */
    . = 1M;
    __link_mem_phys = .;

    /* First put the multiboot header, as it is required to be put very early in the image or the bootloader won't recognize the file format. Next we'll put the .text section. */
    .text BLOCK(4K) : ALIGN(4K)
    {
        __link_mem_code = .;
        *(.multiboot)
        *(.text)
    }

    /* Read-only data. */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        __link_mem_bss = .;
        *(COMMON)
        *(.bss)
        *(.stack)
    }

    /* Extras */
    .extra BLOCK(4k) : ALIGN(4k)
    {
        __link_mem_extra = .;
        *(.note.gnu.build-id)
    }

    /* The compiler might produce some other sections. By default it will put them in a segment with the same name. Simply add stuff here as needed. */
    __link_mem_end = .;
}
