#include "stm32f0xx.h"
#include "lcd.h" // nano_wait
#include "oled.h"

void setup_spi2_oled()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFR12 | GPIO_AFRH_AFR13 | GPIO_AFRH_AFR15);
	GPIOB->MODER = (GPIOB->MODER & ~0xCF000000) | 0x8A000000;
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | SPI_CR1_BR;
	SPI2->CR2 = (SPI2->CR2 & ~SPI_CR2_DS) | SPI_CR2_NSSP | SPI_CR2_SSOE | SPI_CR2_DS_0 | SPI_CR2_DS_3;
	SPI2->CR1 |= SPI_CR1_SPE;
}

static void spi_cmd(int cmd)
{
	// Wait Till Transmit Buffer Empty
	while((SPI2->SR & SPI_SR_TXE) == 0);
	SPI2->DR = cmd;
}

static void spi_data(int data)
{
	// Wait Till Transmit Buffer Empty
	while((SPI2->SR & SPI_SR_TXE) == 0);
	SPI2->DR = data | 0x200;
}

void init_oled()
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

void oled_display1(const char * message)
{
	spi_cmd(0x02);
	int i = 0;
	while(message[i] != 0)
	{
		spi_data(message[i]);
		i++;
	}
}

void oled_display2(const char * message)
{
	spi_cmd(0xc0);
	int i = 0;
	while(message[i] != 0)
	{
		spi_data(message[i]);
		i++;
	}
}

void oled_clear_display1()
{
	oled_display1("                ");
}

void oled_clear_display2()
{
	oled_display2("                ");
}

void oled_cdisplay1(const char * message)
{
	oled_clear_display1();
	oled_display1(message);
}

void oled_cdisplay2(const char * message)
{
	oled_clear_display2();
	oled_display2(message);
}

void oled_clear()
{
	oled_clear_display1();
	oled_clear_display2();
}
