#include "stm32f0xx.h"
#include <stdlib.h>
#include "lcd.h" // nano_wait
#include "i2c.h"

void setup_i2c()
{
	// Configure GPIOB //
	// PB8 = I2C1_SCL
	// PB9 = I2C1_SDA
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	GPIOB->MODER |= 0xA0000;
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFR8 | GPIO_AFRH_AFR9);
	GPIOB->AFR[1] |= (1 << 0) | (1 << 4);

	// Configure I2C1 //
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	I2C1->CR1 &= ~I2C_CR1_PE; // Disable channel
	I2C1->CR1 &= ~(I2C_CR1_ANFOFF | I2C_CR1_ERRIE | I2C_CR1_NOSTRETCH);

	// Configure Clock
	I2C1->TIMINGR = 0;
	I2C1->TIMINGR |= 3 << 20; // SCLDEL = 3
	I2C1->TIMINGR |= 1 << 16; // SCADEL = 1
	I2C1->TIMINGR |= 9 << 0;  // SCLL = 9
	I2C1->TIMINGR |= 3 << 8;  // SCLH = 3

	I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
	I2C1->OAR2 &= ~I2C_OAR2_OA2EN;
	I2C1->CR2 &= ~I2C_CR2_ADD10; // 7 bit addressing mode
	I2C1->CR2 |= I2C_CR2_AUTOEND;

	I2C1->CR1 |= I2C_CR1_PE; // Enable channel
}

void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir)
{
	uint32_t tmpreg = I2C1->CR2;
	tmpreg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD |
			I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START |
			I2C_CR2_STOP);

	if(dir == 1) // Read
	{
		tmpreg |= I2C_CR2_RD_WRN;
	}
	else // Write
	{
		tmpreg &= ~ I2C_CR2_RD_WRN;
	}

	tmpreg |= ((devaddr << 1) & I2C_CR2_SADD) | ((size << 16) & I2C_CR2_NBYTES);
	tmpreg |= I2C_CR2_START;
	I2C1->CR2 = tmpreg;
}

void i2c_stop(void)
{
	if(I2C1->ISR & I2C_ISR_STOPF)
	{
		return;
	}

	// Generate stop bit
	I2C1->CR2 |= I2C_CR2_STOP;

	// Wait until STOPF flag is reset then clear it
	while((I2C1->ISR & I2C_ISR_STOPF) == 0);
	I2C1->ICR |= I2C_ICR_STOPCF;
}

void i2c_waitidle(void)
{
	// Wait until not busy
	while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
}

uint8_t i2c_senddata(uint8_t devaddr, void *pdata, uint8_t size)
{
	if(size <= 0 || pdata == 0)
	{
		return -1;
	}

	uint8_t *udata = (uint8_t*) pdata;
	i2c_waitidle();
	i2c_start(devaddr, size, 0);

	int i;
	for(i = 0; i < size; i++)
	{
		int count = 0;

		// Wait until TXDR register is empty
		while((I2C1->ISR & I2C_ISR_TXIS) == 0)
		{
			count += 1;
			if(count > 1000000)
			{
				return -1;
			}

			if((I2C1->ISR & I2C_ISR_NACKF) != 0)
			{
				I2C1->ICR |= I2C_ICR_NACKCF;
				i2c_stop();
				return -1;
			}
		}
		I2C1->TXDR = udata[i] & I2C_TXDR_TXDATA;
	}

	while((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);

	if((I2C1->ISR & I2C_ISR_NACKF) != 0)
	{
		return -1;
	}

	i2c_stop();
	return 0;
}

uint8_t i2c_recvdata(uint8_t devaddr, uint8_t *pdata, uint8_t size)
{
	if(size <= 0 || pdata == NULL)
	{
		return -1;
	}

	i2c_waitidle();
	i2c_start(devaddr, size, 1);

	int i;
	for(i = 0; i < size; i++)
	{
		while((I2C1->ISR & I2C_ISR_RXNE) == 0);
		pdata[i] = I2C1->RXDR & I2C_RXDR_RXDATA;
	}
	while((I2C1->ISR & I2C_ISR_TC) == 0);

	i2c_stop();
	return 0;
}

void i2c_set_iodir(int value)
{
	uint8_t data[] = {0x0, value};
	i2c_senddata(0x27, data, sizeof(data));
}

int i2c_get_gpio()
{
	uint8_t data[] = {0x09};
	i2c_senddata(0x27, data, sizeof(data));

	uint8_t recvdata[1];
	i2c_recvdata(0x27, recvdata, sizeof(recvdata));
	return recvdata[0];
}

void i2c_write_flash(uint16_t loc, const char *data, uint8_t len)
{
	uint8_t buffer[34];
	buffer[0] = (loc & 0xFF00) >> 8;
	buffer[1] = loc & 0x00FF;

	int i;
	for(i = 0; i < len && i < 32; i++)
	{
		buffer[2 + i] = data[i];
	}

	i2c_senddata(0x57, buffer, len + 2);
}

int i2c_write_flash_complete()
{
	i2c_waitidle();
	i2c_start(0x57, 0, 0);

	while((I2C1->ISR & I2C_ISR_TC) == 0 && (I2C1->ISR & I2C_ISR_NACKF) == 0);
	if((I2C1->ISR & I2C_ISR_NACKF) != 0)
	{
		I2C1->ICR |= I2C_ICR_NACKCF;
		i2c_stop();
		return 0;
	}
	else
	{
		i2c_stop();
		return 1;
	}

	return 0;
}

void i2c_set_gpio(int value)
{
	uint8_t data[] = {0x09, value};
	i2c_senddata(0x27, data, sizeof(data));
}

void i2c_read_flash(uint16_t loc, char data[], uint8_t len)
{
	uint8_t read_request[] = {(loc & 0xFF00) >> 8, loc & 0x00FF};
	i2c_senddata(0x57, read_request, sizeof(read_request));

	i2c_recvdata(0x57, data, len);
}
