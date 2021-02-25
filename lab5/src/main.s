.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

//===================================================================
// ECE 362 Lab Experiment 5
// Timers
//===================================================================

// RCC configuration registers
.equ  RCC,      0x40021000
.equ  AHBENR,   0x14
.equ  GPIOCEN,  0x00080000
.equ  GPIOBEN,  0x00040000
.equ  GPIOAEN,  0x00020000
.equ  APB1ENR,  0x1c
.equ  TIM6EN,   1<<4
.equ  TIM7EN,   1<<5

// NVIC configuration registers
.equ NVIC, 0xe000e000
.equ ISER, 0x100
.equ ICER, 0x180
.equ ISPR, 0x200
.equ ICPR, 0x280
.equ IPR,  0x400
.equ TIM6_DAC_IRQn, 17
.equ TIM7_IRQn, 18

// Timer configuration registers
.equ TIM6, 0x40001000
.equ TIM7, 0x40001400
.equ TIM_CR1,  0x0
.equ TIM_CR2,  0x4
.equ TIM_DIER, 0xc
.equ TIM_SR,   0x10
.equ TIM_EGR,  0x14
.equ TIM_CNT,  0x24
.equ TIM_PSC,  0x28
.equ TIM_ARR,  0x2c

// Timer configuration register bits
.equ TIM_CR1_CEN,  1<<0
.equ TIM_DIER_UDE, 1<<8
.equ TIM_DIER_UIE, 1<<0
.equ TIM_SR_UIF,   1<<0

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

//===========================================================================
// enable_ports  (Autotest 1)
// Enable the RCC clock for GPIO ports B and C.
// Parameters: none
// Return value: none
.global enable_ports
enable_ports:
	push {lr}
	// Student code goes below

	// Enable GPIOC and GPIOB //

	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]

	ldr r2, =GPIOBEN
	orrs r1, r2

	ldr r2, =GPIOCEN
	orrs r1, r2

	str r1, [r0, #AHBENR]

	// GPIOB Configuration //

	// Configures pins PB0 – PB3 to be outputs
	ldr r0, =GPIOB
	ldr r1, [r0, #MODER]

	movs r2, #0xFF // 11 11 11 11
	bics r1, r2

	movs r2, #0x55 // 01 01 01 01
	orrs r1, r2

	// Configures pins PB4 – PB7 to be inputs
	ldr r2, =0xFF00 // 11 11 11 11 00 00 00 00
	bics r1, r2

	str r1, [r0, #MODER]

	// Configures pins PB4 – PB7 to be internally pulled low
	ldr r1, [r0, #PUPDR]

	ldr r2, =0xFF00 // 11 11 11 11 00 00 00 00
	bics r1, r2

	ldr r2, =0xAA00 // 10 10 10 10 00 00 00 00
	orrs r1, r2

	str r1, [r0, #PUPDR]

	// GPIOC Configuration //

	// Configures pins PC0 – PC10 to be outputs
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]

	ldr r2, =0x3FFFFF // 11 11 11 11 11 11 11
	bics r1, r2

	ldr r2, =0x155555 // 01 01 01 01 01 01 01
	orrs r1, r2

	str r1, [r0, #MODER]

	// Student code goes above
	pop  {pc}

//===========================================================================
// Timer 6 Interrupt Service Routine  (Autotest 2)
// Parameters: none
// Return value: none
// Write the entire subroutine below
.global TIM6_DAC_IRQHandler
.type TIM6_DAC_IRQHandler, %function
TIM6_DAC_IRQHandler:
	push {lr}

	// Acknowledge Interrupt //
	ldr r0, =TIM6
	ldr r1, [r0, #TIM_SR]
	ldr r2, =0x0
	str r2, [r0, #TIM_SR]

	// Toggle PC6 //
	ldr r0, =GPIOC
	ldr r1, [r0, #ODR]
	movs r2, #0x40 // 1 0 0 0 0 0 0
	eors r1, r2
	str r1, [r0, #ODR]

	pop {pc}


//===========================================================================
// setup_tim6  (Autotest 3)
// Configure timer 6
// Parameters: none
// Return value: none
.global setup_tim6
setup_tim6:
	push {lr}
	// Student code goes below

	// Enable Clock to TIM6
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]

	movs r2, #0x10 // 1 0 0 0 0
	orrs r1, r2

	str r1, [r0, #APB1ENR]


	// Configure Prescaler
	ldr r0, =TIM6
	ldr r1, [r0, #TIM_PSC]

	ldr r2, =0xFFFF
	bics r1, r2

	ldr r2, =47999
	orrs r1, r2

	str r1, [r0, #TIM_PSC]


	// Configure Couting Period
	ldr r1, [r0, #TIM_ARR]

	ldr r2, =0xFFFF
	bics r1, r2

	ldr r2, =499
	orrs r1, r2

	str r1, [r0, #TIM_ARR]


	// Configure Interrupt Enable Register
	ldr r1, [r0, #TIM_DIER]

	ldr r2, =TIM_DIER_UIE
	orrs r1, r2

	str r1, [r0, #TIM_DIER]


	// Enable Counting
	ldr r1, [r0, #TIM_CR1]

	ldr r2, =TIM_CR1_CEN
	orrs r1, r2

	str r1, [r0, #TIM_CR1]


	// Enable Interrupt
	ldr r2, =1<<TIM6_DAC_IRQn
	ldr r0, =NVIC
	ldr r1, =ISER
	str r2, [r0, r1]

	// Student code goes above
	pop  {pc}

.data
.global display
display: .byte 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07
.global history
history: .space 16
.global offset
offset: .byte 0
.text

//===========================================================================
// show_digit  (Autotest 4)
// Set up the Port C outputs to show the digit for the current
// value of the offset variable.
// Parameters: none
// Return value: none
// Write the entire subroutine below.
.global show_digit
show_digit:
	push {lr}
	ldr r0, =offset
	ldr r1, [r0] // r1 = offset
	movs r2, #7
	ands r1, r2 // r1 = off = offset & 7

	ldr r0, =display
	ldrb r2, [r0, r1] // r2 = display[off]
	lsls r3, r1, #8  // r3 = off << 8

	orrs r3, r2 // (off << 8) | display[off]

	ldr r0, =GPIOC
	str r3, [r0, #ODR]
	pop {pc}

//===========================================================================
// get_cols  (Autotest 5)
// Return the current value of the PC8 - PC4.
// Parameters: none
// Return value: 4-bit result of columns active for the selected row
// Write the entire subroutine below.
.global get_cols
get_cols:
	push {lr}
	ldr r0, =GPIOB
	ldr r1, [r0, #IDR]

	lsrs r1, r1, #4 // r1 = GPIOB->IDR >> 4
	movs r2, #0xf
	ands r1, r2 // r1 = (GPIOB->IDR >> 4) & 0xf

	movs r0, r1 // return r1
	pop {pc}

//===========================================================================
// update_hist  (Autotest 6)
// Update the history byte entries for the current row.
// Parameters: r0: cols: 4-bit value read from matrix columns
// Return value: none
// Write the entire subroutine below.
.global update_hist
update_hist:
	push {r4-r7, lr}

	ldr r1, =offset
	ldr r1, [r1] // r1 = offset
	movs r2, #3
	ands r1, r2 // r1 = row = offset & 3

	movs r2, #0 // r2 = i = 0
for1:
	cmp r2, #4
	bge endfor1

	lsls r3, r1, #2 // r3 = 4 * row
	adds r3, r2     // r3 = 4 * row + i

	ldr r4, =history
	ldrb r5, [r4, r3] // r5 = history[4*row+i]
	lsls r5, r5, #1  // r5 = history[4*row+i] << 1

	movs r6, r0
	lsrs r6, r2 // r6 = cols >> i
	movs r7, #1
	ands r6, r7 // r6 = (cols >> i) & 1

	adds r5, r6 // r5 = history[4*row+i] << 1 + (cols >> i) & 1

	strb r5, [r4, r3]

	adds r2, #1
	b for1
endfor1:

	pop {r4-r7, pc}

//===========================================================================
// set_row  (Autotest 7)
// Set PB3 - PB0 to represent the row being scanned.
// Parameters: none
// Return value: none
// Write the entire subroutine below.
.global set_row
set_row:
	push {lr}
	ldr r0, =offset
	ldr r0, [r0] // r0 = offset

	movs r1, #3
	ands r0, r1 // r0 = row = offset & 3

	movs r1, #1
	lsls r1, r0 // r1 = 1 << row

	ldr r0, =0xf0000
	orrs r0, r1 // r0 = 0xf0000 | 1<<row

	ldr r1, =GPIOB
	str r0, [r1, #BSRR] // GPIOB->BSRR = 0xf0000 | 1<<row

	pop {pc}

//===========================================================================
// Timer 7 Interrupt Service Routine  (Autotest 8)
// Parameters: none
// Return value: none
// Write the entire subroutine below
.global TIM7_IRQHandler
.type TIM7_IRQHandler, %function
TIM7_IRQHandler:
	push {lr}

	// Acknowledge Interrupt //
	ldr r0, =TIM7
	ldr r1, [r0, #TIM_SR]
	ldr r2, =0x0
	str r2, [r0, #TIM_SR]

	bl show_digit
	bl get_cols
	bl update_hist

	// Update Offset //
	ldr r0, =offset
	ldr r1, [r0] // r1 = offset

	adds r1, #1 // r1 = offset + 1
	movs r2, #0x7
	ands r1, r2 // r1 = (offset + 1) & 0x7

	str r1, [r0] // offset = r1

	bl set_row

	pop {pc}
//===========================================================================
// setup_tim7  (Autotest 9)
// Configure Timer 7.
// Parameters: none
// Return value: none
.global setup_tim7
setup_tim7:
	push {lr}
	// Student code goes below


	// Enable RCC Clock for TIM7, Disable for TIM6 //
	ldr r0, =RCC
	ldr r1, [r0, #APB1ENR]

	// Enable TIM7
	movs r2, #0x20 // 1 0 0 0 0 0
	orrs r1, r2

	// Disable TIM6
	movs r2, #0x10 // 1 0 0 0 0
	bics r1, r2

	str r1, [r0, #APB1ENR]


	// Configure Prescaler to 2KHz Signal //
	ldr r0, =TIM7
	ldr r1, [r0, #TIM_PSC]

	ldr r2, =0xFFFF
	bics r1, r2

	ldr r2, =23999
	orrs r1, r2

	str r1, [r0, #TIM_PSC]


	// Configure Auto Reset to 2 Cycles -> 2KHz Signal / 2 = 1 KHz clock //
	ldr r1, [r0, #TIM_ARR]

	ldr r2, =0xFFFF
	bics r1, r2

	ldr r2, =1
	orrs r1, r2

	str r1, [r0, #TIM_ARR]


	// Configure DIER //
	ldr r1, [r0, #TIM_DIER]

	ldr r2, =TIM_DIER_UIE
	orrs r1, r2

	str r1, [r0, #TIM_DIER]

	// Enable Counting //
	ldr r1, [r0, #TIM_CR1]

	ldr r2, =TIM_CR1_CEN
	orrs r1, r2

	str r1, [r0, #TIM_CR1]

	// Enable Interrupt //
	ldr r2, =1<<TIM7_IRQn
	ldr r0, =NVIC
	ldr r1, =ISER
	str r2, [r0, r1]


	// Student code goes above
	pop  {pc}


//===========================================================================
// get_keypress  (Autotest 10)
// Wait for and return the number (0-15) of the ID of a button pressed.
// Parameters: none
// Return value: button ID
.global get_keypress
get_keypress:
	push {lr}
	// Student code goes below
for2:
	wfi
	ldr r0, =offset
	ldr r0, [r0] // r0 = offset

	movs r1, #3
	ands r0, r1 // r0 = offset & 3
	cmp r0, #0
	bne for2 // continue

	movs r0, #0 // r0 = i = 0
for3:
	cmp r0, #16
	bge endfor3

	ldr r1, =history
	ldrb r2, [r1, r0] // r2 = history[i]
	cmp r2, #1
	beq return_i

	adds r0, #1
	b for3
endfor3:


	b for2
endfor2:

return_i:

	// Student code goes above
	pop  {pc}

.global font
font: .byte 0x06, 0x5b, 0x4f, 0x77, 0x66, 0x6d, 0x7d, 0x7c, 0x07, 0x7f, 0x67, 0x39, 0x49, 0x3f, 0x76, 0x5e
//===========================================================================
// handle_key  (Autotest 11)
// Shift the symbols in the display to the left and add a new digit
// in the rightmost digit.
// ALSO: Create your "font" array just above.
// Parameters: ID of new button to display
// Return value: none
.global handle_key
handle_key:
	push {lr}
	// Student code goes below
	movs r1, #0xf
	ands r0, r1 // r0 = key & 0xf

	ldr r2, =display

	movs r1, #0
for4:
	cmp r1, #7
	bge endfor4

	adds r3, r1, #1 // r3 = i + 1
	ldrb r3, [r2, r3] // r3 = display[i + 1]
	strb r3, [r2, r1] // display[i] = display[i + 1]

	adds r1, #1
	b for4
endfor4:

	ldr r1, =font
	ldrb r3, [r1, r0] // r3 = font[key]
	strb r3, [r2, #7] // display[7] = font[key]

	// Student code goes above
	pop  {pc}

.global login
login: .string "bsalha"
.align 2

//===========================================================================
// main
// Already set up for you.
// It never returns.
.global main
main:
	//bl  check_wiring
	bl  autotest
	bl  enable_ports
	bl  setup_tim6
	bl  setup_tim7

endless_loop:
	bl   get_keypress
	bl   handle_key
	b    endless_loop
