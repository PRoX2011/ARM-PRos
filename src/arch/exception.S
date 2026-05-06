.section .text
.align 11
.global exception_vector_table

/*
 * Exception Vector Table (EVT) for ARMv8-A EL1.
 * There are 4 groups of vectors, each with 4 entries (Synchronous, IRQ, FIQ, SError).
 * Each entry is 128 bytes (0x80) apart.
 */

.macro exception_entry label
    .align 7
    b \label
.endm

exception_vector_table:
    /* Group 1: Current EL with SP_EL0 */
    exception_entry el1_sync_invalid
    exception_entry el1_irq_invalid
    exception_entry el1_fiq_invalid
    exception_entry el1_error_invalid

    /* Group 2: Current EL with SP_ELx */
    exception_entry el1_sync
    exception_entry el1_irq
    exception_entry el1_fiq_invalid
    exception_entry el1_error_invalid

    /* Group 3: Lower EL using AArch64 */
    exception_entry el0_sync_invalid
    exception_entry el0_irq_invalid
    exception_entry el0_fiq_invalid
    exception_entry el0_error_invalid

    /* Group 4: Lower EL using AArch32 */
    exception_entry el0_32_sync_invalid
    exception_entry el0_32_irq_invalid
    exception_entry el0_32_fiq_invalid
    exception_entry el0_32_error_invalid

/* Save context: all general-purpose registers x0-x30 */
.macro save_context
    sub sp, sp, #256
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]
    
    /* Ensure SP is 16-byte aligned for C handlers */
    mov x0, sp
    and x0, x0, #0xF
    sub sp, sp, x0
    str x0, [sp, #-16]!     /* Store alignment offset */
.endm

/* Restore context */
.macro restore_context
    ldr x0, [sp], #16       /* Load alignment offset */
    add sp, sp, x0
    
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #16 * 15]
    add sp, sp, #256
.endm

/* Common handler for invalid/unhandled exceptions */
.macro invalid_exception type
    save_context
    mov x0, #\type
    bl handle_invalid_exception
    restore_context
    eret
.endm

el1_sync_invalid:      invalid_exception 0
el1_irq_invalid:       invalid_exception 1
el1_fiq_invalid:       invalid_exception 2
el1_error_invalid:     invalid_exception 3
el0_sync_invalid:      invalid_exception 4
el0_irq_invalid:       invalid_exception 5
el0_fiq_invalid:       invalid_exception 6
el0_error_invalid:     invalid_exception 7
el0_32_sync_invalid:   invalid_exception 8
el0_32_irq_invalid:    invalid_exception 9
el0_32_fiq_invalid:    invalid_exception 10
el0_32_error_invalid:  invalid_exception 11

el1_sync:
    save_context
    bl handle_el1_sync
    restore_context
    eret

el1_irq:
    save_context
    bl handle_el1_irq
    restore_context
    eret

.global enable_interrupts
enable_interrupts:
    msr daifclr, #2
    ret

.global disable_interrupts
disable_interrupts:
    msr daifset, #2
    ret
