#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdbool.h>
#include <stdio.h>

int uart_transmit(char ch, FILE* stream) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = ch;
  
  return 0;
}

ISR(USART0_RX_vect) {
  char data = UDR0;
  printf("%c", data);
}

static FILE uart_output = FDEV_SETUP_STREAM(uart_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void) {
  /* Set baud rate */
  const uint8_t ubrr = ((F_CPU/16/9600)-1);
  UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)ubrr;
  /* Enable receiver, transmitter receiver interrupt */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1 << RXCIE0);
  /* Set frame format: 8 data, 2 stop bit */
  UCSR0C = (3<<UCSZ00);

  stdout = &uart_output;
  
  /* Enable global interrupts */
  sei();
  
  while (true) {}
  
  return 0;
}

