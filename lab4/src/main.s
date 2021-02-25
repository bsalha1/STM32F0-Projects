.syntax unified
.cpu cortex-m0
.fpu softvfp
.thumb

//===================================================================
// ECE 362 Lab Experiment 4
// Interrupts
//===================================================================

// RCC configuration registers
.equ  RCC,      0x40021000
.equ  AHBENR,   0x14
.equ  GPIOCEN,  0x00080000
.equ  GPIOBEN,  0x00040000
.equ  GPIOAEN,  0x00020000
.equ  APB2ENR,  0x18
.equ  SYSCFGCOMPEN, 1

// GPIO configuration registers
.equ  GPIOC,    0x48000800
.equ  GPIOB,    0x48000400
.equ  GPIOA,    0x48000000
.equ  MODER,    0x00
.equ  PUPDR,    0x0c
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28

// SYSCFG constrol registers
.equ SYSCFG, 0x40010000
.equ EXTICR1, 0x8
.equ EXTICR2, 0xc
.equ EXTICR3, 0x10
.equ EXTICR4, 0x14

// External interrupt control registers
.equ EXTI, 0x40010400
.equ IMR, 0
.equ EMR, 0x4
.equ RTSR, 0x8
.equ FTSR, 0xc
.equ SWIER, 0x10
.equ PR, 0x14

// Variables to register things for EXTI on pin 0
.equ EXTI_RTSR_TR0, 1<<0
.equ EXTI_IMR_MR0,  1<<0
.equ EXTI_PR_PR0,   1<<0
// Variables to register things for EXTI on pin 1
.equ EXTI_RTSR_TR1, 1<<1
.equ EXTI_IMR_MR1,  1<<1
.equ EXTI_PR_PR1,   1<<1
// Variables to register things for EXTI on pin 2
.equ EXTI_RTSR_TR2, 1<<2
.equ EXTI_IMR_MR2,  1<<2
.equ EXTI_PR_PR2,   1<<2
// Variables to register things for EXTI on pin 3
.equ EXTI_RTSR_TR3, 1<<3
.equ EXTI_IMR_MR3,  1<<3
.equ EXTI_PR_PR3,   1<<3
// Variables to register things for EXTI on pin 4
.equ EXTI_RTSR_TR4, 1<<4
.equ EXTI_IMR_MR4,  1<<4
.equ EXTI_PR_PR4,   1<<4

// SysTick counter variables...
.equ STK, 0xe000e010
.equ CSR, 0x0
.equ RVR, 0x4
.equ CVR, 0x8

// NVIC configuration registers
.equ NVIC, 0xe000e000
.equ ISER, 0x100
.equ ICER, 0x180
.equ ISPR, 0x200
.equ ICPR, 0x280
.equ IPR,  0x400
.equ EXTI0_1_IRQn,5  // External interrupt number for pins 0 and 1 is IRQ 5.
.equ EXTI2_3_IRQn,6  // External interrupt number for pins 2 and 3 is IRQ 6.
.equ EXTI4_15_IRQn,7 // External interrupt number for pins 4 - 15 is IRQ 7.

// GPIO configuration registers
.equ  GPIOC,    0x48000800
.equ  GPIOB,    0x48000400
.equ  GPIOA,    0x48000000
.equ  MODER,    0x00
.equ  PUPDR,    0x0c
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28

//===========================================================
// gcd
// Euclid's algorithm for Greatest Common Denominator
// Find the GCD of the first two parameters.
// Parameter2 1 and 2 are unsigned integers
// Write the entire subroutine below.
.global gcd
gcd:
	push {lr}
while1:
	cmp r0, r1
	beq endwhile1

if1:
	cmp r0, r1
	bls else1
then1:
	subs r0, r1
	b endif1
else1:
	subs r1, r0
endif1:

	b while1
endwhile1:

	pop {pc}


//===========================================================
// enable_ports
// Enable the RCC clock for GPIO ports A, B, and C.
// Parameters: none
// Write the entire subroutine below.
.global enable_ports
enable_ports:
	push {lr}
	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	ldr r2, =0xC0000

	orrs r1, r2
	str r1, [r0, #AHBENR]
	pop {pc}

//===========================================================
// port_c_output
// Configure PC6, PC7, PC8, and PC9 to be outputs.
// Do not modify any other pin's configuration.
// Parameters: none
// Write the entire subroutine below.
.global port_c_output
port_c_output:
	push {lr}
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]

	// Clear PC6-PC9 settings
	ldr r2, =0xFF000
	bics r1, r2

	// Make PC6-PC9 01=output
	ldr r2, =0x55000
	orrs r1, r2

	str r1, [r0, #MODER]
	pop {pc}

//===========================================================
// port_b_input
// Configure PB2, PB3, and PB4 to be inputs.
// Enable the pull-down resistor for PB2.
// Do not modify any other pin's configuration.
// Parameters: none
// Write the entire subroutine below.
.global port_b_input
port_b_input:
	push {lr}
	ldr r0, =GPIOB

	// Configure
	ldr r1, [r0, #MODER]

	// Clear PB2-PB4 settings 00=input
	ldr r2, =0x3F0 // 11 11 11 00 00
	bics r1, r2
	str r1, [r0, #MODER]

	// Enable Pulldown Resistor
	ldr r1, [r0, #PUPDR]

	// Clear PB2 settings
	ldr r2, =0x30 // 11 00 00
	bics r1, r2

	// Enable PB2 pulldown
	ldr r2, =0x20 // 10 00 00
	orrs r1, r2
	str r1, [r0, #PUPDR]

	pop {pc}

//===========================================================
// toggle_portc_pin
// Change the ODR value from 0 to 1 or 1 to 0 for a specified
// pin of Port C.
// Parameters: r0 holds the pin number to toggle
// Write the entire subroutine below.
.global toggle_portc_pin
toggle_portc_pin:
	push {lr}
	ldr r1, =GPIOC
	ldr r2, [r1, #ODR]

	movs r3, #1
	lsls r3, r0
	eors r2, r3

	str r2, [r1, #ODR]

	pop {pc}

//===========================================================
// SysTick_Handler
// The ISR for the SysTick interrupt.
// Call toggle_portc_pin(7).
// Parameters: none
//
.global SysTick_Handler
.type SysTick_Handler, %function
SysTick_Handler:
	push {lr}
	// Student code goes below
	movs r0, #7
	bl toggle_portc_pin
	// Student code goes above
	pop  {pc}

//===========================================================
// enable_systick
// Enable the SysTick interrupt to occur every 0.5 seconds.
// Parameters: none
.global enable_systick
enable_systick:
	push {lr}
	// Student code goes below
	ldr r0, =STK

	// Set CLKSOURCE to 0 (6 MHz clock)
	// Turn on TICKINT and ENABLE
	ldr r1, [r0, #CSR]

	movs r2, #0x4 // 100
	bics r1, r2   // 0--

	movs r2, #0x3 // 011
	orrs r1, r2
	str r1, [r0, #CSR]

	// Set STK_CVR to 0
	ldr r1, [r0, #CVR]

	ldr r2, =0xFFFFFF
	bics r1, r2 // clear bottom 24 bits
	str r1, [r0, #CVR]

	// Set STK_RVR to work at 2 Hz with the 6 MHz clock
	ldr r1, [r0, #RVR]

	ldr r2, =0xFFFFFF
	bics r1, r2 // clear bottom 24 bits

	ldr r2, =2999999 // set to half the clock's frequency 3 MHz
	orrs r1, r2
	str r1, [r0, #RVR]

	// Student code goes above
	pop  {pc}

//===========================================================
// Write the EXTI interrupt handler for pins 2 and 3 below.
// Copy the name from startup/startup_stm32.s, create a label
// of that name below, declare it to be global, and declare
// it to be a function.
// It acknowledge the pending bit for pin 3, and it should
// call toggle_portc_pin(8)
.global EXTI2_3_IRQHandler
.type EXTI2_3_IRQHandler, %function
EXTI2_3_IRQHandler:
	push {lr}
	ldr r0, =EXTI
	ldr r1, =EXTI_PR_PR3
	str r1, [r0, #PR]

	movs r0, #8
	bl toggle_portc_pin

	pop {pc}

//===========================================================
// Write the EXTI interrupt handler for pins 4-15 below.
// It should acknowledge the pending bit for pin4, and it
// should call toggle_portc_pin(9).
.global EXTI4_15_IRQHandler
.type EXTI4_15_IRQHandler, %function
EXTI4_15_IRQHandler:
	push {lr}
	ldr r0, =EXTI
	ldr r1, =EXTI_PR_PR4
	str r1, [r0, #PR]

	movs r0, #9
	bl toggle_portc_pin

	pop {pc}

//===========================================================
// enable_exti
// Enable the SYSCFG subsystem, and select Port B for
// pins 2, 3, and 4
// Parameters: none
.global enable_exti
enable_exti:
	push {lr}
	// Student code goes below

	// Enable SYSCFG
	ldr r0, =RCC
	ldr r1, [r0, #APB2ENR]
	ldr r2, =SYSCFGCOMPEN

	orrs r1, r2
	str r1, [r0, #APB2ENR]

	// Enable EXTI2-4
	ldr r0, =SYSCFG
	ldr r1, [r0, #EXTICR1]

	ldr r2, =0xFF00 // 1111 1111 0000 0000
	bics r1, r2
	ldr r2, =0x1100 // 0001 0001 0000 0000
	orrs r1, r2
	str r1, [r0, #EXTICR1]

	ldr r1, [r0, #EXTICR2]
	ldr r2, =0x000F // 0000 0000 0000 1111
	bics r1, r2
	ldr r2, =0x0001 // 0000 0000 0000 0001
	orrs r1, r2
	str r1, [r0, #EXTICR2]

	// Student code goes above
	pop  {pc}


//===========================================================
// init_rtsr
// Configure the EXTI_RTSR register so that an EXTI
// interrupt is generated on the rising edge of
// pins 2, 3, and 4.
// Parameters: none
.global init_rtsr
init_rtsr:
	push {lr}
	// Student code goes below
	ldr r0, =EXTI_RTSR_TR2
	ldr r1, =EXTI_RTSR_TR3
	ldr r2, =EXTI_RTSR_TR4
	orrs r0, r1
	orrs r0, r2

	ldr r1, =EXTI
	ldr r2, [r1, #RTSR]

	orrs r2, r0
	str r2, [r1, #RTSR]
	// Student code goes above
	pop  {pc}

///==========================================================
// init_imr
// Configure the EXTI_IMR register so that the EXTI
// interrupts are unmasked for pins 2, 3, and 4.
// Parameters: none
.global init_imr
init_imr:
	push {lr}
	// Student code goes below
	ldr r0, =EXTI_IMR_MR2
	ldr r1, =EXTI_IMR_MR3
	ldr r2, =EXTI_IMR_MR4
	orrs r0, r1
	orrs r0, r2

	ldr r1, =EXTI
	ldr r2, [r1, #IMR]

	orrs r2, r0
	str r2, [r1, #IMR]
	// Student code goes above
	pop  {pc}

//===========================================================
// init_iser
// Enable the two interrupts for EXTI pins 2-3 and EXTI pins 4-15.
// Do not enable any other interrupts.
// Parameters: none
.global init_iser
init_iser:
	push {lr}
	// Student code goes below
	ldr r0,=1 << EXTI2_3_IRQn
	ldr r1,=1 << EXTI4_15_IRQn
	orrs r0, r1

    ldr r1, =NVIC
    ldr r2, =ISER
    str r0, [r1, r2]
	// Student code goes above
	pop  {pc}

//===========================================================
// adjust_priorities
// Set the priority for the EXTI pins 2-3 interrupt to 192
// Set the priority for the EXTI pins 4-15 interrupt to 128
// Do not adjust the priority for any other interrupts.
.global adjust_priorities
adjust_priorities:
	push {lr}
	// Student code goes below
	ldr r0, =NVIC
	ldr r1, =IPR
	adds r1, #4 // r1 = IPR1

	ldr r2, [r0, r1]
	ldr r3, =0xFFFF0000
	bics r2, r3
	ldr r3, =0x80C00000
	orrs r2, r3

	str r2, [r0, r1]
	// Student code goes above
	pop  {pc}

//===========================================================
// The main subroutine calls everything else.
// It never returns.
.global login
login: .string "bsalha" // Change to your login
.align 2
.global main
main:
	bl autotest // Uncomment when most things are working
	ldr  r0,=3000000000 // 3 billion
	ldr  r1,=750000000  // 750 million
	bl   gcd            // find the GCD
	// CHECK: Result in R0 now should be 750 million
	ldr  r0,=1125000000
	ldr  r1,=3000000000
	bl   gcd
	// CHECK: Result in R0 now should be 375 million

	bl enable_ports
	bl port_c_output
	bl port_b_input
	bl enable_systick

	bl enable_exti
	bl init_rtsr
	bl init_imr
	bl adjust_priorities
	bl init_iser

endless_loop:
	movs r0,#6
	bl   toggle_portc_pin
	ldr  r0,=4000000000
	ldr  r1,=1000
	bl   gcd
	b    endless_loop
