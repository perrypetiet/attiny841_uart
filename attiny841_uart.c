/*
 * attiny841_uart.c
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
/******************************* INCLUDES ********************************/

#include "attiny841_uart.h"
#include <avr/interrupt.h>

/******************************* GLOBAL VARIABLES ************************/

UART_DATA* uart_data[MAX_UART_INTERFACES] = {NULL};

/******************************* LOCAL FUNCTIONS *************************/

/*
 * USART data register empty interrupt USART0
 */
ISR(USART0_UDRE_vect)
{
  if (uart_data[USART_0]->tx.tail != uart_data[USART_0]->tx.head)
  {
    uint8_t tail = uart_data[USART_0]->tx.tail;
    if (uart_data[USART_0]->tx.tail == DATA_BUFFER_LEN - 1)
    {
      uart_data[USART_0]->tx.tail = 0;
    } 
    else 
    {
      uart_data[USART_0]->tx.tail++;
    }
    UDR0 = uart_data[USART_0]->tx.data[tail];
  }
  else
  {
    UCSR0B &= ~(1 << UDRIE0);  
  }
}

/*
 * USART RX interrupt USART0
 */
ISR(USART0_RX_vect)
{
  uart_data[USART_0]->rx.data[uart_data[USART_0]->rx.head] = UDR0;
  if (uart_data[USART_0]->rx.head == DATA_BUFFER_LEN - 1)
  {
    uart_data[USART_0]->rx.head = 0;
  }
  else 
  {
    uart_data[USART_0]->rx.head++;
  }
}

/*
 * USART data register empty interrupt USART1
 */
ISR(USART1_UDRE_vect)
{
  if (uart_data[USART_1]->tx.tail != uart_data[USART_1]->tx.head)
  {
    uint8_t tail = uart_data[USART_1]->tx.tail;
    if (uart_data[USART_1]->tx.tail == DATA_BUFFER_LEN - 1)
    {
      uart_data[USART_1]->tx.tail = 0;
    } 
    else
    {
      uart_data[USART_1]->tx.tail++;
    }
    UDR1 = uart_data[USART_1]->tx.data[tail];
  }
  else
  {
    UCSR1B &= ~(1 << UDRIE1);
  }
}

/*
 * USART RX interrupt USART0
 */
ISR(USART1_RX_vect)
{
  uart_data[USART_1]->rx.data[uart_data[USART_1]->rx.head] = UDR1;
  if (uart_data[USART_1]->rx.head == DATA_BUFFER_LEN - 1)
  {
    uart_data[USART_1]->rx.head = 0;
  }
  else
  {
    uart_data[USART_1]->rx.head++;
  }
}

/******************************* GLOBAL FUNCTIONS ************************/

/*
 * This functions initializes a UART interface. Creates buffers and sets
 * correct register values for given settings.
 *
 * IN: uart_select | USART0 or USART1                                      
 * IN: baudRate    | Perhipheral baudrate
 * IN: byteSize    | Byte format
 * IN: parity      | Parity, see defines in header
 * IN: stopBits    | Amount of stopbits, 1 or 2
 *
 * RETURNS bool: True if init successful. 
 */
bool init_uart(uint8_t  uart_select,
               uint32_t baudRate, 
               uint8_t  byteSize, 
               uint8_t  parity, 
               uint8_t  stopBits)
{
  bool init_succes = false;
  if (uart_data[uart_select] == NULL)
  {
    if ((parity == 0 || parity == 1)     &&
        (byteSize >= 5 && byteSize <= 8) &&   
        (stopBits == 1 || stopBits == 2))
    {
      uart_data[uart_select] = malloc(sizeof(UART_DATA)); 
      //disable interrupts
      cli();
      if (uart_select == USART_0)
      {
        //set to async mode
        UCSR0B |= (1 << UMSEL00 | 1 << UMSEL01);

        //set baudrate
        uint16_t ubrrRegister = F_CPU / 16 / baudRate - 1;
        UBRR0H = (unsigned char) (ubrrRegister>>8);
        UBRR0L = (unsigned char)  ubrrRegister;

        //set bytesize
        switch(byteSize)
        {
          case 5:
            UCSR0B &= ~(1 << UCSZ02);
            UCSR0C &= ~(1 << UCSZ01 | 1 << UCSZ00);
            break;
          case 6:
            UCSR0B &= ~(1 << UCSZ02);
            UCSR0C &= ~(1 << UCSZ01);
            UCSR0C |= (1 << UCSZ00);
            break;
          case 7:
            UCSR0B &= ~(1 << UCSZ02);
            UCSR0C |= (1 << UCSZ01);
            UCSR0C &= ~(1 << UCSZ00);
            break;
          case 8:
            UCSR0B &= ~(1 << UCSZ02);
            UCSR0C |= (1 << UCSZ01 | 1 << UCSZ00);
            break;
        }

        //set parity
        switch(parity)
        {
          case PARITY_NONE:
            UCSR0C &= ~(1 << UPM00 | 1 << UPM01);
            break;
          case PARITY_EVEN:
            UCSR0C &= ~(1 << UPM00);
            UCSR0C |= (1 << UPM01);
            break;
          case PARITY_ODD:
            UCSR0C |= (1 << UPM01 | 1 << UPM00);
            break;
        }

        //set stopbits
        switch(stopBits)
        {
          case 1:
            UCSR0C &= ~(1 << USBS0);
            break;
          case 2:
            UCSR0C |= (1 << USBS0);
            break;
        }
        //enable receiver and transmitter
        UCSR0B |= (1 << TXEN0 | 1 << RXEN0 | 1 << RXCIE0);
        UCSR0B &= ~(1 << TXCIE0);

        init_succes = true;
      }

      else if (uart_select == USART_1)
      {
        //set to async mode
        UCSR1B |= (1 << UMSEL10 | 1 << UMSEL11);

        //set baudrate
        uint16_t ubrrRegister = F_CPU / 16 / baudRate - 1;
        UBRR1H = (unsigned char) (ubrrRegister>>8);
        UBRR1L = (unsigned char)  ubrrRegister;

        //set bytesize
        switch(byteSize)
        {
          case 5:
            UCSR1B &= ~(1 << UCSZ12);
            UCSR1C &= ~(1 << UCSZ11 | 1 << UCSZ10);
            break;
          case 6:
            UCSR1B &= ~(1 << UCSZ12);
            UCSR1C &= ~(1 << UCSZ11);
            UCSR1C |= (1 << UCSZ10);
            break;
          case 7:
            UCSR1B &= ~(1 << UCSZ12);
            UCSR1C |= (1 << UCSZ11);
            UCSR1C &= ~(1 << UCSZ10);
            break;
          case 8:
            UCSR1B &= ~(1 << UCSZ12);
            UCSR1C |= (1 << UCSZ11 | 1 << UCSZ10);
            break;
        }

        //set parity
        switch(parity)
        {
          case PARITY_NONE:
            UCSR1C &= ~(1 << UPM10 | 1 << UPM11);
            break;
          case PARITY_EVEN:
            UCSR1C &= ~(1 << UPM10);
            UCSR1C |= (1 << UPM11);
            break;
          case PARITY_ODD:
            UCSR1C |= (1 << UPM11 | 1 << UPM10);
            break;
        }

        //set stopbits
        switch(stopBits)
        {
          case 1:
            UCSR1C &= ~(1 << USBS1);
            break;
          case 2:
            UCSR1C |= (1 << USBS1);
            break;
        }
        //enable receiver and transmitter
        UCSR1B |= (1 << TXEN1 | 1 << RXEN1 | 1 << RXCIE1);
        UCSR1B &= ~(1 << TXCIE1);

        init_succes = true;
      }
      sei();    

      uart_data[uart_select]->rx.head = 0;
      uart_data[uart_select]->rx.tail = 0;
      uart_data[uart_select]->tx.head = 0;
      uart_data[uart_select]->tx.tail = 0;
    }
  }
  return init_succes;
}

/*
 * This functions deinitializes a UART interface. Frees buffers and 
 * disables peripheral interrupts.
 *
 * IN: uart_select | USART0 or USART1                                      
 *
 * RETURNS bool: True if deinit successful. 
 */
bool deinit_uart(uint8_t uart_select)
{
  if(uart_data[uart_select] != NULL)
  {
    switch(uart_select)
    {
      case USART_0:
        UCSR0B &= ~(1 << TXEN0 | 1 << RXEN0 | 1 << RXCIE0);
        break;
      case USART_1:
        UCSR1B &= ~(1 << TXEN1 | 1 << RXEN1 | 1 << RXCIE1);
        break;
    }
    free(uart_data[uart_select]);
    uart_data[uart_select] = NULL;
    return true;
  }
  return false;
}

/*
 * Transmits data over UART peripheral. Give data pointer and length 
 * in bytes.
 *
 * IN: uart_select | USART0 or USART1                                      
 * IN: data*       | Pointer to beginning of data.
 * IN: data_len    | Length of data.
 *
 * RETURNS bool: True if transmit succes.
 */
bool uart_transmit(uint8_t uart_select, uint8_t* data, uint8_t data_len)
{
  bool transmit_success = false;

  if (uart_data[uart_select] != NULL)
  {
    if (data_len < DATA_BUFFER_LEN - uart_data[uart_select]->tx.head)
    {
      memcpy(&uart_data[uart_select]->tx.data[uart_data[uart_select]->tx.head], data, data_len);
      uart_data[uart_select]->tx.head += data_len;
    } else
    {
      uint8_t overflow = data_len - (DATA_BUFFER_LEN - uart_data[uart_select]->tx.head);

      memcpy(&uart_data[uart_select]->tx.data[uart_data[uart_select]->tx.head], 
             data, 
             data_len - overflow);
      memcpy(&uart_data[uart_select]->tx.data[0], 
             &data[data_len - overflow], 
             overflow);
      uart_data[uart_select]->tx.head = overflow;
    }

    uint8_t tail = uart_data[uart_select]->tx.tail;
    if (uart_data[uart_select]->tx.tail == DATA_BUFFER_LEN - 1)
    {
      uart_data[uart_select]->tx.tail = 0;
    }
    else
    {
      uart_data[uart_select]->tx.tail++;
    }
    if (uart_select == USART_0)
    {
      UDR0 = uart_data[uart_select]->tx.data[tail];
      UCSR0B |= (1 << UDRIE0);
    }
    else if (uart_select == USART_1)
    {
      UDR1 = uart_data[uart_select]->tx.data[tail];
      UCSR1B |= (1 << UDRIE1);
    }
    
  }
  return transmit_success;
}

/*
 * This functions reads a received byte from the RX buffer if 
 * it is available.
 *
 * IN:  uart_select | USART0 or USART1.                                      
 * OUT: byte        | address where received byte should be stored.
 *
 * RETURNS bool: True if a byte was taken.
 */
bool uart_read_byte(uint8_t uart_select, uint8_t* byte)
{
  if (uart_data[uart_select] != NULL)
  {
    if (uart_data[uart_select]->rx.tail != uart_data[uart_select]->rx.head)
    {
      *byte = uart_data[uart_select]->rx.data[uart_data[uart_select]->rx.tail];
      if (uart_data[uart_select]->rx.tail == DATA_BUFFER_LEN - 1)
      {
        uart_data[uart_select]->rx.tail = 0;
      }
      else 
      {
        uart_data[uart_select]->rx.tail++;
      }
      return true;
    }
  }
  return false;
}

/******************************* THE END *********************************/
