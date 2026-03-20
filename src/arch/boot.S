.section .text
.global _start

_start:
    ldr x30, =stack_top
    mov sp, x30
    bl main

.loop:
    wfe
    b .loop