
//============================================================================
// ECE 362 lab experiment 8 -- SPI and DMA
//============================================================================

#include "stm32f0xx.h"
#include "lcd.h"
#include <stdio.h> // for sprintf()

// Be sure to change this to your login...
const char login[] = "bsalha";

// Prototypes for misc things in lcd.c
void nano_wait(unsigned int);

// Write your subroutines below.


void setup_bb()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER = (GPIOB->MODER & ~0xCF000000) | 0x45000000;
	GPIOB->ODR = (GPIOB->ODR & ~0x2000) | 0x1000;
}

void small_delay()
{
	nano_wait(10000000);
}

void bb_write_bit(int bit)
{
	GPIOB->ODR = (bit == 1) ? (GPIOB->ODR | 0x8000) : (GPIOB->ODR & ~0x8000);
	small_delay();
	GPIOB->ODR |= 0x2000;
	small_delay();
	GPIOB->ODR &= ~0x2000;
}

void bb_write_byte(int byte)
{
	bb_write_bit((byte >> 7) & 1);
	bb_write_bit((byte >> 6) & 1);
	bb_write_bit((byte >> 5) & 1);
	bb_write_bit((byte >> 4) & 1);
	bb_write_bit((byte >> 3) & 1);
	bb_write_bit((byte >> 2) & 1);
	bb_write_bit((byte >> 1) & 1);
	bb_write_bit((byte >> 0) & 1);
}

void bb_cmd(int cmd)
{
	GPIOB->ODR &= ~0x1000;
	small_delay();
	bb_write_bit(0);
	bb_write_bit(0);
	bb_write_byte(cmd);
	small_delay();
	GPIOB->ODR |= 0x1000;
	small_delay();
}

void bb_data(int data)
{
	GPIOB->ODR &= ~0x1000;
	small_delay();
	bb_write_bit(1);
	bb_write_bit(0);
	bb_write_byte(data);
	small_delay();
	GPIOB->ODR |= 0x1000;
	small_delay();
}

void bb_init_oled()
{
	nano_wait(1000000);
	bb_cmd(0x38);
	bb_cmd(0x08);
	bb_cmd(0x01);
	nano_wait(2000000);
	bb_cmd(0x06);
	bb_cmd(0x02);
	bb_cmd(0x0c);
}

void bb_display1(const char * message)
{
	bb_cmd(0x02);
	int i = 0;
	while(message[i] != 0)
	{
		bb_data(message[i]);
		i++;
	}
}

void bb_display2(const char * message)
{
	bb_cmd(0xc0);
	int i = 0;
	while(message[i] != 0)
	{
		bb_data(message[i]);
		i++;
	}
}

void setup_spi2()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFR12 | GPIO_AFRH_AFR13 | GPIO_AFRH_AFR15);
	GPIOB->MODER = (GPIOB->MODER & ~0xCF000000) | 0x8A000000;
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_BR;
	SPI2->CR2 = (SPI2->CR2 & ~SPI_CR2_DS) | SPI_CR2_NSSP | SPI_CR2_SSOE | SPI_CR2_DS_0 | SPI_CR2_DS_3;
	SPI2->CR1 |= SPI_CR1_SPE;
}

void spi_cmd(int cmd)
{
	// Wait Till Transmit Buffer Empty
	while((SPI2->SR & SPI_SR_TXE) == 0);
	SPI2->DR = cmd;
}

void spi_data(int data)
{
	// Wait Till Transmit Buffer Empty
	while((SPI2->SR & SPI_SR_TXE) == 0);
	SPI2->DR = data | 0x200;
}

void spi_init_oled()
{
	nano_wait(1000000);
	spi_cmd(0x38);
	spi_cmd(0x08);
	spi_cmd(0x01);
	nano_wait(2000000);
	spi_cmd(0x06);
	spi_cmd(0x02);
	spi_cmd(0x0c);
}

void spi_display1(const char * message)
{
	spi_cmd(0x02);
	int i = 0;
	while(message[i] != 0)
	{
		spi_data(message[i]);
		i++;
	}
}

void spi_display2(const char * message)
{
	spi_cmd(0xc0);
	int i = 0;
	while(message[i] != 0)
	{
		spi_data(message[i]);
		i++;
	}
}

void spi_enable_dma(short * buffer)
{
	RCC->AHBENR |= RCC_AHBENR_DMAEN;
	DMA1_Channel5->CPAR = &(SPI2->DR);
	DMA1_Channel5->CMAR = buffer;
	DMA1_Channel5->CNDTR = 34;
	DMA1_Channel5->CCR |= DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_CIRC;
	DMA1_Channel5->CCR = (DMA1_Channel5->CCR & ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE)) | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0;
	DMA1_Channel5->CCR |= DMA_CCR_EN;
	SPI2->CR2 |= SPI_CR2_TXDMAEN;
}

void setup_spi1()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFR4 | GPIO_AFRL_AFR5 | GPIO_AFRL_AFR7);
	GPIOA->MODER = (GPIOA->MODER & ~0xCFF0) | 0x8A50;

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 = (SPI1->CR1 & ~SPI_CR1_BR) | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_MSTR;
	SPI1->CR2 = (SPI1->CR2 & ~SPI_CR2_DS) | SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2;
	SPI1->CR1 |= SPI_CR1_SPE;
}





// Write your subroutines above.

void show_counter(short buffer[])
{
    for(int i=0; i<10000; i++) {
        char line[17];
        sprintf(line,"% 16d", i);
        for(int b=0; b<16; b++)
            buffer[1+b] = line[b] | 0x200;
    }
}

void internal_clock();
void demo();
void autotest();

extern const Picture *image;

int main(void)
{
    //internal_clock();
//	demo();
//    autotest();

    setup_bb();
    bb_init_oled();
    bb_display1("Hello,");
    bb_display2(login);

    setup_spi2();
    spi_init_oled();
    spi_display1("Hello again,");
    spi_display2(login);

    short buffer[34] = {
            0x02, // This word sets the cursor to the beginning of line 1.
            // Line 1 consists of spaces (0x20)
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0xc0, // This word sets the cursor to the beginning of line 2.
            // Line 2 consists of spaces (0x20)
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
            0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
    };

    spi_enable_dma(buffer);
    show_counter(buffer);

    setup_spi1();
    LCD_Init();
    LCD_Clear(BLACK);
    LCD_DrawLine(10,20,100,200, WHITE);
    LCD_DrawRectangle(10,20,100,200, GREEN);
    LCD_DrawFillRectangle(120,20,220,200, RED);
    LCD_Circle(50, 260, 50, 1, BLUE);
    LCD_DrawFillTriangle(130,130, 130,200, 190,160, YELLOW);
    LCD_DrawChar(150,155, BLACK, WHITE, 'X', 16, 1);
    LCD_DrawString(140,60,  WHITE, BLACK, "ECE 362", 16, 0);
    LCD_DrawString(140,80,  WHITE, BLACK, "has the", 16, 1);
    LCD_DrawString(130,100, BLACK, GREEN, "best toys", 16, 0);
    LCD_DrawPicture(110,220,(const Picture *)&image);
}
