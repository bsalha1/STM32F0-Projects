void setup_usart5();

int simple_putchar(int value);

int better_putchar(int value);

int interrupt_getchar();

int __io_putchar(int ch);

int __io_getchar(void);

void USART3_4_5_6_7_8_IRQHandler(void);

void enable_tty_interrupt(void);
