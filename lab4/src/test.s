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
