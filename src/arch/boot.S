.section .text
.global _start

_start:
    mrs x0, CurrentEL
    and x0, x0, #12
    lsr x0, x0, #2

    cmp x0, #3
    b.ne .L_not_el3

    mov x0, #0x301
    msr scr_el3, x0
    mov x0, #0x3c9
    msr spsr_el3, x0
    adr x0, .L_not_el3
    msr elr_el3, x0
    
    msr cptr_el3, xzr
    
    eret

.L_not_el3:
    mrs x0, CurrentEL
    and x0, x0, #12
    lsr x0, x0, #2
    
    cmp x0, #2
    b.ne .L_in_el1

    msr cptr_el2, xzr
    mrs x0, cnthctl_el2
    orr x0, x0, #3
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr

    mov x0, #(1 << 31)
    orr x0, x0, #(1 << 1)
    msr hcr_el2, x0

    mov x0, #0x3c5
    msr spsr_el2, x0
    adr x0, .L_in_el1
    msr elr_el2, x0
    eret

.L_in_el1:
    mov x0, #0x300000
    msr cpacr_el1, x0
    isb sy

    mrs x0, mpidr_el1
    and x0, x0, #0xff
    cbz x0, 1f
0:
    wfe
    b   0b
1:
    ldr x30, =stack_top
    mov sp, x30

    ldr x0, =exception_vector_table
    msr vbar_el1, x0
    isb sy

    ldr x0, =__bss_start
    ldr x1, =__bss_end
2:
    cmp x0, x1
    b.hs 3f
    str xzr, [x0], #8
    b   2b
3:
    bl  main

4:
    wfe
    b   4b