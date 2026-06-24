#ifndef DRIVERS_UART_H
#define DRIVERS_UART_H

#include<stdint.h>

struct RingBuffer{
    volatile uint8_t buffer[4096]; //Buffer/Буфер
    volatile int head; //End buffer/Конец буфер
    volatile int tail; //Current index buffer/Текущий индекс в буфере
};

extern struct RingBuffer RxBuffer;

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);
char uart_getc(void);

/*
    This function should only be used for interrupt handlers.
    Функция должна использоваться только для обработчика прерываний
*/
void uart_gets_interrupt(); 

#endif
