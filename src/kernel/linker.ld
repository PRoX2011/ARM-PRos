ENTRY(_start)

SECTIONS
{
    . = 0x40000000;

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    . = ALIGN(8);
    . = . + 0x4000; /* 16 KB stack */
    stack_top = .;
}
