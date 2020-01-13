    .syntax unified
    .cpu cortex-m4
    .thumb
.text

.global max7219_send

.equ RCC_AHB2ENR,   0x4002104C
.equ GPIOB_MODER,   0x48000400
.equ GPIOB_OTYPER,  0x48000404
.equ GPIOB_OSPEEDR, 0x48000408
.equ GPIOB_PUPDR,   0x4800040C
.equ GPIOB_ODR,     0x48000414

.thumb_func
GPIO_init:
    // Enable AHB2 clock for GPIOB
    ldr     r0,     =RCC_AHB2ENR
    ldr     r1,     [r0]
    orr     r1,     r1,     #0x02
    str     r1,     [r0]

    // Set GPIO PB3,4,5 as output
    ldr     r0,     =GPIOB_MODER
    ldr     r1,     [r0]
    and     r1,     r1,     #0xFFFFF03F
    orr     r1,     r1,     #0x540
    str     r1,     [r0]

    mov     r1,     #0xA800
    ldr     r0,     =GPIOB_OSPEEDR
    strh    r1,     [r0]
    bx lr

.thumb_func
max7219_send:
    push    {r4-r7}
    rbit    r0,     r0
    rbit    r1,     r1
    lsr     r0,     r0,     #0x18
    lsr     r1,     r1,     #0x10
    orr     r0,     r0,     r1
    movs    r4,     r0
    movs    r6,     #0x0F
loop_send:
    mov     r5,     #0x00
    and     r7,     r4,     #0x01
    orr     r5,     r5,     r7
    lsl     r7,     r5,     #0x03

    ldr     r0,     =GPIOB_ODR
    str     r7,     [r0]

    eor     r5,     r5,     #0x04
    lsl     r7,     r5,     #0x03
    str     r7,     [r0]

    lsr     r4,     #0x01
    subs    r6,     #0x01
    bge     loop_send
loop_send_end:
    ldr     r0,     =GPIOB_ODR
    mov     r5,     #0x10
    str     r5,     [r0]
    mov     r5,     #0x30
    str     r5,     [r0]
    pop     {r4-r7}
    bx      lr

