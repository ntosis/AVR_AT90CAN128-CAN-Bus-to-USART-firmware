#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
/*
 *  constants and macros
 */

/* size of RX/TX buffers */
#define UART_RX0_BUFFER_MASK ( UART_RX0_BUFFER_SIZE - 1)


#define UART_TX0_BUFFER_MASK ( UART_TX0_BUFFER_SIZE - 1)


#if ( UART_RX0_BUFFER_SIZE & UART_RX0_BUFFER_MASK )
	#error RX0 buffer size is not a power of 2
#endif
#if ( UART_TX0_BUFFER_SIZE & UART_TX0_BUFFER_MASK )
	#error TX0 buffer size is not a power of 2
#endif



	/* TLS-Added 48P/88P/168P/328P */
	/* ATmega with one USART */
	#define ATMEGA_USART0
	#define UART0_RECEIVE_INTERRUPT   USART_RX_vect
	#define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
	#define UART0_STATUS   UCSR0A
	#define UART0_CONTROL  UCSR0B
	#define UART0_DATA     UDR0
	#define UART0_UDRIE    UDRIE0


/*
 *  Module global variables
 */

#if defined( USART0_ENABLED )
	#if defined( ATMEGA_USART ) || defined( ATMEGA_USART0 )
		static volatile uint8_t UART_TxBuf[UART_TX0_BUFFER_SIZE];
		static volatile uint8_t UART_RxBuf[UART_RX0_BUFFER_SIZE];
		
		#if defined( USART0_LARGE_BUFFER )
			static volatile uint16_t UART_TxHead;
			static volatile uint16_t UART_TxTail;
			static volatile uint16_t UART_RxHead;
			static volatile uint16_t UART_RxTail;
			static volatile uint8_t UART_LastRxError;
		#else
			static volatile uint8_t UART_TxHead;
			static volatile uint8_t UART_TxTail;
			static volatile uint8_t UART_RxHead;
			static volatile uint8_t UART_RxTail;
			static volatile uint8_t UART_LastRxError;
		#endif
		
	#endif
#endif





/*************************************************************************
Function: uart0_init()
Purpose:  initialize UART and set baudrate
Input:    baudrate using macro UART_BAUD_SELECT()
Returns:  none
**************************************************************************/
void uart0_init()
{
	//uint16_t baudrate
	UART_TxHead = 0;
	UART_TxTail = 0;
	UART_RxHead = 0;
	UART_RxTail = 0;

	/* 1 M-bit with 16 MHZ Crystal
	  UCSR0A = (1<< U2X0);
	  UBRR0H = 0;		// set baud rate
	  UBRR0L = 1; */
	  // UBRR0L = 0; 2 MBIT
		UBRR0H = 0;
	   UBRR0L = 51; //19200 baudrate optional for testing
	  /* Enable Tx and Rx and INTerupt */
	  UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);

	  /* Set Frame: Data 8 Bit, No Parity and 1 Stop Bit */
	 // UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); ATMEGA128P

	  /* Set Frame: Data 8 Bit, No Parity and 1 Stop Bit */
	    UCSR0C = (0<<UMSEL0)|(1<<UCSZ01)|(1<<UCSZ00);


} /* uart0_init */


/*************************************************************************
Function: uart0_getc()
Purpose:  return byte from ringbuffer
Returns:  lower byte:  received byte from ringbuffer
          higher byte: last receive error
**************************************************************************/
uint16_t uart0_getc(void)
{
	uint16_t tmptail;
	uint8_t data;

	if ( UART_RxHead == UART_RxTail ) {
		return UART_NO_DATA;   /* no data available */
	}

	/* calculate /store buffer index */
	tmptail = (UART_RxTail + 1) & UART_RX0_BUFFER_MASK;
	UART_RxTail = tmptail;

	/* get data from receive buffer */
	data = UART_RxBuf[tmptail];

	return (UART_LastRxError << 8) + data;

} /* uart0_getc */

/*************************************************************************
Function: uart0_peek()
Purpose:  Returns the next byte (character) of incoming UART data without
          removing it from the ring buffer. That is, successive calls to
		  uartN_peek() will return the same character, as will the next
		  call to uartN_getc()
Returns:  lower byte:  next byte in ring buffer
          higher byte: last receive error
**************************************************************************/
uint16_t uart0_peek(void)
{
	uint16_t tmptail;
	uint8_t data;

	if ( UART_RxHead == UART_RxTail ) {
		return UART_NO_DATA;   /* no data available */
	}

	tmptail = (UART_RxTail + 1) & UART_RX0_BUFFER_MASK;

	/* get data from receive buffer */
	data = UART_RxBuf[tmptail];

	return (UART_LastRxError << 8) + data;

} /* uart0_peek */

/*************************************************************************
Function: uart0_putc()
Purpose:  write byte to ringbuffer for transmitting via UART
Input:    byte to be transmitted
Returns:  none
**************************************************************************/

int uart0_putch(uint8_t ch, FILE *stream)
{
   if (ch == '\n')
    uart0_putch('\r', stream);

   while (!(UCSR0A & (1<<UDRE0)));
   UDR0=ch;

   return 0;
}

/*************************************************************************
Function: uart0_available()
Purpose:  Determine the number of bytes waiting in the receive buffer
Input:    None
Returns:  Integer number of bytes in the receive buffer
**************************************************************************/
int uart0_available(void)
{
	return (UART_RX0_BUFFER_MASK + UART_RxHead - UART_RxTail) % UART_RX0_BUFFER_MASK;
} /* uart0_available */

/*************************************************************************
Function: uart0_flush()
Purpose:  Flush bytes waiting the receive buffer.  Acutally ignores them.
Input:    None
Returns:  None
**************************************************************************/
void uart0_flush(void)
{
	UART_RxHead = UART_RxTail;
}

/* uart0_flush */
ISR(USART0_RX_vect)
{
			    uint16_t tmphead;
			    uint8_t data;
			    uint8_t usr;
			    uint8_t lastRxError;

			    /* read UART status register and UART data register */
			    usr  = UART0_STATUS;
			    data = UART0_DATA;

			    lastRxError = (usr & (_BV(FE0)|_BV(DOR0)) );

			    /* calculate buffer index */
			    tmphead = ( UART_RxHead + 1) & UART_RX0_BUFFER_MASK;

			    if ( tmphead == UART_RxTail ) {
			        /* error: receive buffer overflow */
			        lastRxError = UART_BUFFER_OVERFLOW >> 8;
			    } else {
			        /* store new index */
			        UART_RxHead = tmphead;
			        /* store received data in buffer */
			        UART_RxBuf[tmphead] = data;
			    }
			    UART_LastRxError = lastRxError;
}
