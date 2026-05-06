.section .text
.global _start

_start:
    /* Check current Exception Level */
    mrs x0, CurrentEL
    and x0, x0, #12
    lsr x0, x0, #2

    /* If in EL3, drop to EL2 */
    cmp x0, #3
    b.ne .L_not_el3

    /* EL3 setup */
    mov x0, #0x301          /* RW=1 (EL2 AArch64), NS=1 (Non-secure) */
    msr scr_el3, x0
    mov x0, #0x3c9          /* D=1, A=1, I=1, F=1, M=EL2h */
    msr spsr_el3, x0
    adr x0, .L_not_el3
    msr elr_el3, x0
    
    /* Disable EL3 traps for SIMD/FP */
    msr cptr_el3, xzr
    
    eret

.L_not_el3:
    mrs x0, CurrentEL
    and x0, x0, #12
    lsr x0, x0, #2
    
    /* If in EL2, drop to EL1 */
    cmp x0, #2
    b.ne .L_in_el1

    /* EL2 setup */
    msr cptr_el2, xzr       /* Disable EL2 traps for FP/SIMD */
    mrs x0, cnthctl_el2
    orr x0, x0, #3
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr

    /* Set EL1 to AArch64 */
    mov x0, #(1 << 31)      /* RW=1 (AArch64) */
    orr x0, x0, #(1 << 1)   /* SWIO=1 */
    msr hcr_el2, x0

    mov x0, #0x3c5          /* D=1, A=1, I=1, F=1, M=EL1h */
    msr spsr_el2, x0
    adr x0, .L_in_el1
    msr elr_el2, x0
    eret

.L_in_el1:
    /* Final EL1 setup */
    mov x0, #0x300000       /* FPEN=0b11 (Full access) */
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

    /* Set up Exception Vector Table */
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