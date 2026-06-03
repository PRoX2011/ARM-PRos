ENTRY(_start)

/* Raspberry Pi 3 (AArch64): firmware loads kernel8.img at 0x80000 */
SECTIONS
{
    . = 0x80000;

    .text : ALIGN(4096) {
        *(.text*)
    }

    .rodata : ALIGN(4096) {
        *(.rodata*)
    }

    .data : ALIGN(4096) {
        *(.data*)
    }

    .bss : ALIGN(8) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    }

    . = ALIGN(16);
    . = . + 0x10000;
    stack_top = .;

    __end = .;
}
