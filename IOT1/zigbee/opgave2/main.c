#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdbool.h>
#include <stdio.h>

static int seconds_gone = 0;

int uart_transmit(char ch, FILE* stream) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = ch;
  
  return 0;
}

ISR(TIMER1_COMPA_vect) {
  printf("Seconds gone: %i\r\n", seconds_gone++);
}

static FILE uart_output = FDEV_SETUP_STREAM(uart_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void) {
  /* Set baud rate */
  const uint8_t ubrr = ((F_CPU/16/9600)-1);
  UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)ubrr;
  /* Enable receiver and transmitter */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  /* Set frame format: 8 data, 2 stop bit */
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);
  
  /* Set CTC mode: WGM12 = 1 */
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10); /* Set prescaler to 1024 */
  /* Calculate OCR1A for 1-second interval
	 F_CPU = 16 MHz, prescaler = 1024
	 Timer counts = 16,000,000 / 1024 = 15625 counts per second */
  OCR1A = 15625;
  /* Enable Timer1 Compare A Match Interrupt */
  TIMSK1 |= (1 << OCIE1A); 

  /* Enable global interrupts */
  sei();

  stdout = &uart_output;
  
  while (true) {}
  
  return 0;
}

