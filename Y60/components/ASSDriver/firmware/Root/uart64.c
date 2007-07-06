#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include "uart64.h"
#include "defines.h"


void uart0_init(void){
#define UART_BAUD_CALC( theBaudRate, theClock) \
    ( (theClock) / ((theBaudRate) * 16l) - 1)
#define UART_BAUD_CALC_DS( theBaudRate, theClock) \
    ( (theClock) / ((theBaudRate) * 8l) - 1)

   if((g_DIPSwitch&_BV(DIP_ETHERNET_PORT)) == 0){
    	//double speed mode: U2X = 1
//        UBRR0H = (uint8_t)(UART_BAUD_CALC_DS( UART0_BAUD_RATE_HS,F_CPU ) >> 8);
//        UBRR0L = (uint8_t)UART_BAUD_CALC_DS( UART0_BAUD_RATE_HS,F_CPU );
        switch(g_BaudRateFactor){
            case 0:
                UBRR0H = 0;
                UBRR0L = (F_CPU/(UART0_BAUD_RATE_HS*8)-1) & 0xFF;
                break;
            case 1:
                UBRR0H = 0;
                UBRR0L = (F_CPU/(2*UART0_BAUD_RATE_HS*8)-1) & 0xFF;
                break;
            case 2:
                UBRR0H = 0;
                UBRR0L = (F_CPU/(4*UART0_BAUD_RATE_HS*8)-1) & 0xFF;
                break;
            case 3:
                UBRR0H = 0;
                UBRR0L = (F_CPU/(8*UART0_BAUD_RATE_HS*8)-1) & 0xFF;
                break;
            case 4:
                UBRR0H = 0;
                UBRR0L = (F_CPU/(16*UART0_BAUD_RATE_HS*8)-1) & 0xFF;
                break;
        }
        // turn on double speed
        UCSR0A = _BV(U2X0);
        // Enable receiver and transmitter, also turn on RX complete interrupt
        UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
        //asynchronous 8N1
        UCSR0C = (3 << UCSZ00);
        g_UART_mode = HS;
    }else{
    	//double speed mode: U2X = 1
        UBRR0H = (uint8_t)(UART_BAUD_CALC_DS( UART0_BAUD_RATE_LS,F_CPU ) >> 8);
        UBRR0L = (uint8_t)UART_BAUD_CALC_DS( UART0_BAUD_RATE_LS,F_CPU );
        // turn on double speed
        UCSR0A = _BV(U2X0);
        // Enable receiver and transmitter, also turn on RX complete interrupt
        UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
        //asynchronous 8N1
        UCSR0C = (3 << UCSZ00);
        g_UART_mode = LS;
    }
}

int uart0_putchar(char c, FILE *stream){
    if (c == '\a'){
        fputs("*ring*\n", stderr);
        return 0;
    }

    if (c == '\n')
        uart0_putchar('\r', stream);
        loop_until_bit_is_set(UCSR0A, UDRE0);
        UDR0 = c;

    return 0;
}



/*
 * Receive a character from the UART Rx.
 *
 * This features a simple line-editor that allows to delete and
 * re-edit the characters entered, until either CR or NL is entered.
 * Printable characters entered will be echoed using uart_putchar().
 *
 * Editing characters:
 *
 * . \b (BS) or \177 (DEL) delete the previous character
 * . ^u kills the entire input buffer
 * . ^w deletes the previous word
 * . ^r sends a CR, and then reprints the buffer
 * . \t will be replaced by a single space
 *
 * All other control characters will be ignored.
 *
 * The internal line buffer is RX_BUFSIZE (80) characters long, which
 * includes the terminating \n (but no terminating \0).  If the buffer
 * is full (i. e., at RX_BUFSIZE-1 characters in order to keep space for
 * the trailing \n), any further input attempts will send a \a to
 * uart_putchar() (BEL character), although line editing is still
 * allowed.
 *
 * Input errors while talking to the UART will cause an immediate
 * return of -1 (error indication).  Notably, this will be caused by a
 * framing error (e. g. serial line "break" condition), by an input
 * overrun, and by a parity error (if parity was enabled and automatic
 * parity recognition is supported by hardware).
 *
 * Successive calls to uart_getchar() will be satisfied from the
 * internal buffer until that buffer is emptied again.
 */
int
uart0_getchar(FILE *stream)
{
  uint8_t c;
  char *cp, *cp2;
  static char b[RX0_BUFSIZE];
  static char *rxp;

  if (rxp == 0)
    for (cp = b;;)
      {
	loop_until_bit_is_set(UCSR0A, RXC0);
	if (UCSR0A & _BV(FE0))
	  return _FDEV_EOF;
	if (UCSR0A & _BV(DOR0))
	  return _FDEV_ERR;
	c = UDR0;
	/* behaviour similar to Unix stty ICRNL */
	if (c == '\r')
	  c = '\n';
	if (c == '\n')
	  {
	    *cp = c;
	    uart0_putchar(c, stream);
	    rxp = b;
	    break;
	  }
	else if (c == '\t')
	  c = ' ';

	if ((c >= (uint8_t)' ' && c <= (uint8_t)'\x7e') ||
	    c >= (uint8_t)'\xa0')
	  {
	    if (cp == b + RX0_BUFSIZE - 1)
	      uart0_putchar('\a', stream);
	    else
	      {
		*cp++ = c;
		uart0_putchar(c, stream);
	      }
	    continue;
	  }

	switch (c)
	  {
	  case 'c' & 0x1f:
	    return -1;

	  case '\b':
	  case '\x7f':
	    if (cp > b)
	      {
		uart0_putchar('\b', stream);
		uart0_putchar(' ', stream);
		uart0_putchar('\b', stream);
		cp--;
	      }
	    break;

	  case 'r' & 0x1f:
	    uart0_putchar('\r', stream);
	    for (cp2 = b; cp2 < cp; cp2++)
	      uart0_putchar(*cp2, stream);
	    break;

	  case 'u' & 0x1f:
	    while (cp > b)
	      {
		uart0_putchar('\b', stream);
		uart0_putchar(' ', stream);
		uart0_putchar('\b', stream);
		cp--;
	      }
	    break;

	  case 'w' & 0x1f:
	    while (cp > b && cp[-1] != ' ')
	      {
		uart0_putchar('\b', stream);
		uart0_putchar(' ', stream);
		uart0_putchar('\b', stream);
		cp--;
	      }
	    break;
	  }
      }

  c = *rxp++;
  if (c == '\n')
    rxp = 0;

  return c;
}

