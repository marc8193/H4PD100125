#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdbool.h>
#include <stdio.h>

#include "lib/ssd1306.h"

char ring_buffer[256] = {0};
volatile uint8_t head = 0;
volatile uint8_t tail = 0;

int uart_transmit(char ch, FILE* stream) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = ch;
  
  return 0;
}

ISR(USART_RX_vect) {
  char data = UDR0;
  ring_buffer[head++] = data;
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
  
  uint8_t display_status = SSD1306_Init(SSD1306_ADDR);
  if (display_status != SSD1306_SUCCESS) {
    return 0;
  }

  SSD1306_ClearScreen();

  /* Enable global interrupts */
  sei();

  while (true) {
    if (head != tail) {
        char ch = ring_buffer[tail++];
        cli();
        SSD1306_DrawChar(ch, NORMAL);
        sei();
    }
  }  
  return 0;
}
