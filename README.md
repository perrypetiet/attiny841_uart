# attiny841_uart

Library for using the USART functionality of the attiny841.
Can run USART1 aswell as USART0 in parralel.

Counts on 8Mhz cpu clock. Not every baudrate is compatible with every
clock speed. Consult datasheet! If using a different clock speed,
hange the define: F_CPU
 
Depends on some standard AVR includes.
 
This library doesn't account for overflowing of the buffers!
Unintended behaviour is expected when buffers are not emptied in time.
