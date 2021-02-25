#include "stm32f0xx.h"
#include <stdlib.h>

void nano_wait(unsigned int n);

void setup_i2c();

void i2c_start(uint32_t devaddr, uint8_t size, uint8_t dir);

void i2c_stop(void);

void i2c_waitidle(void);

uint8_t i2c_senddata(uint8_t devaddr, void *pdata, uint8_t size);

uint8_t i2c_recvdata(uint8_t devaddr, uint8_t *pdata, uint8_t size);

void i2c_set_iodir(int value);

int i2c_get_gpio();

void i2c_write_flash(uint16_t loc, const char *data, uint8_t len);

int i2c_write_flash_complete();

void i2c_set_gpio(int value);

void i2c_read_flash(uint16_t loc, char data[], uint8_t len);
