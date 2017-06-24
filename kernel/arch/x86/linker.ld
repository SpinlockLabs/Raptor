/* Bootloader looks at this image and starts at the symbol marked as the entrypoint. */
ENTRY(_start)

SECTIONS
{
    /* Begin putting sections at 1 MiB, a conventional place for kernels to be loaded by the bootloader. */
    . = 1M;

    /* First put the multiboot header, as it is required to be put very early in the image or the bootloader won't recognize the file format. Next we'll put the .text section. */
    .text BLOCK(4K) : ALIGN(4K)
    {
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
        *(COMMON)
        *(.bss)
    }

    /* The compiler might produce some other sections. By default it will put them in a segment with the same name. Simply add stuff here as needed. */
}
