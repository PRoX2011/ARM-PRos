ENTRY(_start)

/* Raspberry Pi 3 (AArch64): firmware loads kernel8.img at 0x80000 */
SECTIONS
{
    . = 0x80000;

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : ALIGN(8) {
        __bss_start = .;
        *(.bss)
        . = ALIGN(8);
        __bss_end = .;
    }

    . = ALIGN(16);
    . = . + 0x4000; /* 16 KB stack */
    stack_top = .;

    __end = .;
}
