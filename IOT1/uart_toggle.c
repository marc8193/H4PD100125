#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

char line[256] = {0};
int line_cursor = 0;

ISR(USART_RX_vect)
{
    char data = UDR0;
	if (data == '\r' || data == '\n') {
		printf("\r\n");
		
		char* addr_str = strtok(line, ",");
		char* pin_str = strtok(NULL, ",");
		if (addr_str && pin_str) {
			uint8_t* addr = (uint8_t*)strtol(addr_str, NULL, 16);
			uint8_t pin = (uint8_t)strtol(pin_str, NULL, 10);
			*addr ^= (1 << pin);
		}

		memset(line, 0, sizeof(line));
		line_cursor = 0;

    } else {
		line[line_cursor] = data;
		line_cursor++;

        printf("%c", data);
    }
}

int uart_transmit(char ch, FILE* stream)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = ch;
    
	return 0;
}

static FILE uart_output = FDEV_SETUP_STREAM(uart_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void) {
    UBRR0 = 103;  // 9600 baud @ 16 MHz
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);  // Enable RX + + TX + RX interrupt
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data

    sei();	// Enable global interrupts
	
	stdout = &uart_output;
	
    while (1) {}

    return 0;
}

