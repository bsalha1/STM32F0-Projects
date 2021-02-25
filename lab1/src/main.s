.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.global login
login: .asciz "bsalha"

.align 2
.global main
main:
    bl   autotest // Uncomment this ONLY when you're done.
    movs r0, #1
    movs r1, #2
    movs r2, #4
    movs r3, #8
    bl   example
    nop

	// Step 3.1
    movs r0, #35
    movs r1, #18
    movs r2, #23
    movs r3, #12
    bl   step31
    nop

	// Step 3.2
    movs r0, #10
    movs r1, #3
    movs r2, #18
    movs r3, #42
    bl   step32
    nop

	// Step 3.3
    movs r0, #24
    movs r1, #35
    movs r2, #52
    movs r3, #85
    bl   step33
    nop

	// Step 4.1
    movs r0, #29
    movs r1, #42
    movs r2, #93
    movs r3, #184
    bl   step41
    nop

	// Step 4.2
    movs r0, #0xaa
    bl   step42
    nop

	// Step 5.1
    movs r0, #0
    movs r1, #16
    movs r2, #2
    movs r3, #3
    bl   step51
    nop

    // Step 5.2
    movs r0, #5
    bl   step52
    nop


    bl   setup_portc
loop:
    bl   toggle_leds
    ldr  r0, =500000000
wait:
    subs r0,#83
    bgt  wait
    b    loop

.global example
example:
	// R0 = R1 + R2 + R3
    adds r0,r1
    adds r0,r2
    adds r0,r3
    bx   lr

.global step31
step31:
	// R0 = (R1 + R2)*(R0 - R3)
	subs r0,r3
	adds r1,r2
	muls r0,r1,r0
	bx   lr

.global step32
step32:
	// R0 = ((-R0)+(R3-R1))*R2
	subs r3, r1
	rsbs r0, #0
	adds r0, r3
	muls r0, r2, r0
    bx   lr

.global step33
step33:
	// R0 = R3 - (R2 - (R1 - R0))
	subs r1, r0
	subs r2, r1
	subs r3, r2
	movs r0, r3
    bx   lr

.global step41
step41:
	// R0 = (R0 | R1) ^ (R0 & R2 | R3)
	orrs r1, r0
	ands r2, r0
	orrs r2, r3
	eors r1, r2
	movs r0, r1
    bx   lr

.global step42
step42:
    // R0 = (R0 & 0xf0) | 0x10
    movs r1, #0xf0
    movs r2, #0x10
    ands r0, r1
    orrs r0, r2
    bx   lr

.global step51
step51:
    // R0 = (R3 << R1) >> R2
    lsls r3, r1
    lsrs r3, r2
    movs r0, r3
    bx   lr

.global step52
step52:
    // R0 = ((R0 & ~0x1) << 3) | 5
    movs r1, #0x1
    bics r0, r1
    lsls r0, #3

    movs r1, #5
    orrs r0, r1
    bx   lr

// Step 6: Type in the .equ constant initializations below
.equ RCC, 0x40021000
.equ AHBENR, 0x14
.equ GPIOCEN, 0x00080000
.equ GPIOC, 0x48000800
.equ MODER, 0x00
.equ ODR, 0x14
.equ ENABLE6_TO_9, 0x55000
.equ PINS6_TO_9, 0x3c0

.global setup_portc
setup_portc:
    ldr r0, =RCC
    ldr r1, [r0, #AHBENR]
    ldr r2, =GPIOCEN
    orrs r1, r2
    str r1, [r0, #AHBENR]

    ldr r0, =GPIOC
    ldr r1, [r0, #MODER]
    ldr r2, =ENABLE6_TO_9
    orrs r1, r2
    str r1, [r0, #MODER]

    bx   lr

.global toggle_leds
toggle_leds:
    ldr r0, =GPIOC
    ldr r1, [r0, #ODR]
    ldr r2, =PINS6_TO_9
    eors r1, r2
    str r1, [r0, #ODR]

    bx   lr
