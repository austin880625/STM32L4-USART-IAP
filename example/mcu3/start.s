    .syntax unified
    .cpu cortex-m4
    .thumb

.section .isr_vector,"ax",%progbits
    .global Reset_Handler
    .global Default_Handler

    .macro IRQ handler
    .word \handler
    .weak \handler
    .set \handler, Default_Handler
    .endm

_vectors:
    .word _estack
    .word Reset_Handler
    IRQ NMI_Handler
    IRQ HardFault_Handler
    IRQ MemManageFault_Handler
    IRQ BusFault_Handler
    IRQ UsageFault_Handler
    //1C to 28 are reserved
    .word 0
    .word 0
    .word 0
    .word 0
    IRQ SVCall_Handler
    .word 0
    .word 0
    IRQ PendSV_Handler
    IRQ SysTick_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0
    IRQ EXTI0_Handler
    IRQ EXTI1_Handler
    IRQ EXTI2_Handler
    IRQ EXTI3_Handler
    IRQ EXTI4_Handler
    .fill 0x188-(.-_vectors), 1, 0x0

    .thumb_func
Default_Handler:
    bx lr
    .thumb_func
Reset_Handler:
data_copy:
    ldr r1, __sidata
    ldr r3, __edata
    ldr r2, __sdata
    subs r3, r3, r2
    beq bss_zero
data_copy_loop:
    ldrb r4, [r1], #1
    strb r4, [r2], #1
    subs r3, r3, #1
    bgt data_copy_loop

bss_zero:
    ldr r1, __sbss
    ldr r3, __ebss
    subs r3, r3, r1
    mov r2, #0
    beq _run
bss_zero_loop:
    strb r2, [r1], #1
    subs r3, r3, #1
    bgt bss_zero_loop
_run:
    bl main
_program_end:
    b _program_end

__sidata: .word _sidata
__sdata: .word _sdata
__edata: .word _edata
__sbss: .word _sbss
__ebss: .word _ebss
