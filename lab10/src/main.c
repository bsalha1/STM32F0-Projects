
//============================================================================
// ECE 362 lab experiment 10 -- Asynchronous Serial Communication
//============================================================================

#include "stm32f0xx.h"
#include "ff.h"
#include "diskio.h"
#include "fifo.h"
#include "tty.h"
#include <string.h> // for memset()
#include <stdio.h> // for printf()

void advance_fattime(void);
void command_shell(void);

// Write your subroutines below.

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

int simple_getchar(void)
{
	// Wait till RX register not empty
	while((USART5->ISR & USART_ISR_RXNE) == 0);

	// Return value read
	return USART5->RDR & 0x1FF;
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

int better_getchar(void)
{
	int read = line_buffer_getchar();
	return read == '\r' ? '\n' : read;
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

void setup_spi1(void)
{
	// Configure GPIO //

	// PA5-7 = SPI1
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	// Enable AF0 for PA5-7
	// Enable output for PA1
	GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODER1) | GPIO_MODER_MODER1_0;
	GPIOA->MODER &= ~((0x3 << (2 * 5)) | (0x3 << (2 * 6)) | (0x3 << (2 * 7)));
	GPIOA->MODER |=   (0x2 << (2 * 5)) | (0x2 << (2 * 6)) | (0x2 << (2 * 7));
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFR5 | GPIO_AFRL_AFR6 | GPIO_AFRL_AFR7);

	// Enable pull-up for PA6
	GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPDR15) | (0x1 << (2 * 6));


	// Configure SPI1 //
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 &= ~SPI_CR1_SPE;
	SPI1->CR1 |= SPI_CR1_BR; // lowest baud rate
	SPI1->CR1 &= ~(SPI_CR1_BIDIOE | SPI_CR1_BIDIOE);
	SPI1->CR1 |= SPI_CR1_MSTR;
	SPI1->CR2 |= SPI_CR2_NSSP;
	SPI1->CR2 |= (SPI1->CR2 & ~SPI_CR2_DS) | (0x7 << 8); // 8-bit word size
	SPI1->CR2 |= SPI_CR2_FRXTH;
	SPI1->CR1 |= SPI_CR1_SPE;
}

void spi_high_speed(void)
{
	SPI1->CR1 &= ~SPI_CR1_SPE;
	SPI1->CR1 = (SPI1->CR1 & ~SPI_CR1_BR) | (0x4 << 3);
	SPI1->CR1 |= SPI_CR1_SPE;
}

void TIM14_IRQHandler(void)
{
	TIM14->SR &= ~TIM_SR_UIF;
	advance_fattime();
}

void setup_tim14(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
	TIM14->PSC = 47999;
	TIM14->ARR = 1999;
	TIM14->DIER |= TIM_DIER_UIE;
	TIM14->CR1 |= TIM_CR1_CEN;

	NVIC->ISER[0] = 1 << TIM14_IRQn;
}

// Write your subroutines above.

const char testline[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\r\n";

int main()
{
    setup_usart5();

    // Uncomment these when you're asked to...
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);

    // Test 2.2 simple_putchar()
    //
//    for(;;)
//        for(const char *t=testline; *t; t++)
//            simple_putchar(*t);

    // Test for 2.3 simple_getchar()
    //
//    for(;;)
//        simple_putchar( simple_getchar() );

    // Test for 2.4 and 2.5 __io_putchar() and __io_getchar()
    //
//    printf("Hello!\n");
//    for(;;)
//        putchar( getchar() );

    // Test for 2.6
    //
//    for(;;) {
//        printf("Enter string: ");
//        char line[100];
//        fgets(line, 99, stdin);
//        line[99] = '\0'; // just in case
//        printf("You entered: %s", line);
//    }

    // Test for 2.7
    //
//    enable_tty_interrupt();
//    for(;;) {
//        printf("Enter string: ");
//        char line[100];
//        fgets(line, 99, stdin);
//        line[99] = '\0'; // just in case
//        printf("You entered: %s", line);
//    }

    // Test for 2.8 Test the command shell and clock.
    //
    enable_tty_interrupt();
    setup_tim14();
    FATFS fs_storage;
    FATFS *fs = &fs_storage;
    f_mount(fs, "", 1);
    command_shell();

    return 0;
}
