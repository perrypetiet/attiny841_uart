
#include "attiny841_uart.h"


/*
 * Simple echo program for both UART interfaces.
 */
int main(void)
{
  init_uart(USART_1, 9600, 8, PARITY_NONE, 1);
  init_uart(USART_0, 9600, 8, PARITY_NONE, 1);
 
  /* Replace with your application code */
  while (1) 
  {
    uint8_t byte;
    if (uart_read_byte(USART_1, &byte))
    {
      uart_transmit(USART_1, &byte, 1);
    }
    if (uart_read_byte(USART_0, &byte))
    {
      uart_transmit(USART_0, &byte, 1);
    }
  }
}

