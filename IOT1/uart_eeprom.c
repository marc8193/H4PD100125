#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char line[256] = {0};
int line_cursor = 0;

long EEMEM eeprom_seconds;

ISR(USART_RX_vect)
{
    char data = UDR0;
	if (data == '\r' || data == '\n') {
		printf("\r\n");

		long seconds = strtol(line, NULL, 10);
		if (seconds > 0) {
			OCR1A = 15625 * seconds;
			TIMSK1 |= (1 << OCIE1A); // Enable Timer1 Compare A Match Interrupt

			printf("Changed to %li seconds.\r\n", seconds);
			eeprom_write_block(&seconds, &eeprom_seconds, sizeof(seconds));
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

ISR(TIMER1_COMPA_vect)
{
	long seconds = 0;
	eeprom_read_block(&seconds, &eeprom_seconds, sizeof(seconds));
	printf("Current timer is %li seconds.\r\n", seconds);
}

static FILE uart_output = FDEV_SETUP_STREAM(uart_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void) {
    UBRR0 = 103;  // 9600 baud @ 16 MHz
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);  // Enable RX + + TX + RX interrupt
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data

    TCCR1B |= (1 << WGM12); // Set CTC mode: WGM12 = 1
    TCCR1B |= (1 << CS12) | (1 << CS10); // Set prescaler to 1024
	long seconds = 0;
	eeprom_read_block(&seconds, &eeprom_seconds, sizeof(seconds));
	// Calculate OCR1A for 1-second interval
    // F_CPU = 16 MHz, prescaler = 1024
    // Timer counts = 16,000,000 / 1024 = 15625 counts per second
	OCR1A = 15625 * seconds; // 0..15624 = 15625 counts
    TIMSK1 |= (1 << OCIE1A); // Enable Timer1 Compare A Match Interrupt

    sei();	// Enable global interrupts

	stdout = &uart_output;

    while (1) {}

    return 0;
}

