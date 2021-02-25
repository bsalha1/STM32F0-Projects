#include "stm32f0xx.h"
#include "fifo.h"
#include "tty.h"
#include "usart.h"

void setup_usart5()
{
	// Configure GPIO //
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;

	// Set PC12 = USART5_TX
	GPIOC->MODER |= 0x2 << (2 * 12);
	GPIOC->AFR[1] |= 0x2 << (4 * 4);

	// Set PD2 = USART5_RX
	GPIOD->MODER |= 0x2 << (2 * 2);
	GPIOD->AFR[0] |= 0x2 << (4 * 2);


	// Configure USART //
	RCC->APB1ENR |= RCC_APB1ENR_USART5EN;
	USART5->CR1 &= ~USART_CR1_UE;
	USART5->CR1 &= ~((1 << 28) | (1 << 12)); // Set word size = 8
	USART5->CR2 &= ~USART_CR2_STOP; // 1 stop bit
	USART5->CR1 &= ~USART_CR1_PCE;  // disable parity checking
	USART5->CR1 &= ~USART_CR1_OVER8; // over-sample by 16
	USART5->BRR = (USART5->BRR & ~0xFFFF) | 0x1A1; // 115.2 KBps Baud rate
	USART5->CR1 |= (USART_CR1_TE | USART_CR1_RE); // Enable TX and RX
	USART5->CR1 |= USART_CR1_UE; // Enable USART5

	// Wait for RX enable to take effect
	while((USART5->ISR & USART_ISR_REACK) == 0);

	// Wait for TX enable to take effect
	while((USART5->ISR & USART_ISR_TEACK) == 0);
}

int simple_putchar(int value)
{
	// Wait till current TX is done
	while((USART5->ISR & USART_ISR_TXE) == 0);

	// Write out to TDR
	USART5->TDR = (USART5->TDR & ~0x1FF) | value;

	return value;
}

int better_putchar(int value)
{
	if(value == '\n')
	{
		simple_putchar('\r');
	}

	simple_putchar(value);

	return value;
}

int interrupt_getchar()
{
	while(fifo_newline(&input_fifo) == 0)
	{
		asm volatile ("wfi");
	}

	return fifo_remove(&input_fifo);
}

int simple_getchar(void)
{
	// Wait till RX register not empty
	while((USART5->ISR & USART_ISR_RXNE) == 0);

	// Return value read
	return USART5->RDR & 0x1FF;
}

int __io_putchar(int ch)
{
	return better_putchar(ch);
}

int __io_getchar(void)
{
	return interrupt_getchar();
}

void USART3_4_5_6_7_8_IRQHandler(void)
{
	if(USART5->ISR & USART_ISR_ORE)
	{
		USART5->ICR |= USART_ICR_ORECF;
	}

	int character = USART5->RDR & 0x1FF;
	if(fifo_full(&input_fifo))
	{
		return;
	}

	insert_echo_char(character);
}

void enable_tty_interrupt(void)
{
	// Enable USART5 Interrupt on RX not empty
	USART5->CR1 |= USART_CR1_RXNEIE;
	NVIC->ISER[0] = (1 << 29);
}
