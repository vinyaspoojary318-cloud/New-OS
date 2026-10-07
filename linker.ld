/* Linker script for a higher-half x86-64 kernel loaded by Limine */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

SECTIONS
{
    /* Higher half starting address required by Limine protocol */
    . = 0xffffffff80000000;

    .text : {
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : {
        *(COMMON)
        *(.bss .bss.*)
    }

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
