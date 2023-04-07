/*
 * attiny841_uart.h
 *
 * Library for using the USART functionality of the attiny841.
 * Can run USART1 aswell as USART0 in parralel
 *
 * Counts on 8Mhz cpu clock. Not every baudrate is compatible with every
 * clock speed. Consult datasheet! If using a different clock speed,
 * change the define: F_CPU
 *
 * Depends on some standard AVR includes.
 *
 * This library doesn't account for overflowing of the buffers!
 * Unintended behaviour is expected when buffers are not emptied in time.
 *
 * Created: 7-4-2023 13:23:16
 * Author:  Perry Petiet
 */ 
#define F_CPU 8000000UL

#ifndef ATTINY841_UART_H_
#define ATTINY841_UART_H_

/******************************* INCLUDES ********************************/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <avr/io.h>


/******************************* DEFINES *********************************/
#define MAX_UART_INTERFACES 2
#define USART_0             0
#define USART_1             1

#define DATA_BUFFER_LEN 16

#define PARITY_NONE 0
#define PARITY_ODD  1                         
#define PARITY_EVEN 2

/******************************* TYPEDEFS ********************************/

typedef struct
{
  uint8_t data[DATA_BUFFER_LEN];                    /* Pointer to data  */
  uint8_t head;                                     /* Head of buffer   */
  uint8_t tail;                                     /* Tail of buffer   */
} DATA_BUFFER;

typedef struct
{
  DATA_BUFFER rx;                                    /* Receive buffer   */
  DATA_BUFFER tx;                                    /* Transmit buffer  */

} UART_DATA;

/******************************* LOCAL FUNCTIONS *************************/

/******************************* GLOBAL FUNCTIONS ************************/

bool init_uart(uint8_t  uart_select,
               uint32_t baudRate, 
               uint8_t  byteSize, 
               uint8_t  parity, 
               uint8_t  stopBits);

bool deinit_uart(uint8_t uart_select);

bool uart_transmit(uint8_t uart_select, uint8_t* data, uint8_t data_len);

bool uart_read_byte(uint8_t uart_select, uint8_t* byte);


/******************************* THE END *********************************/
               
#endif /* AVR_UART_H_ */