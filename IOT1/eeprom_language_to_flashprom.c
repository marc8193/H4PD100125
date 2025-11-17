#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char flashprom_languages[][255] PROGMEM = {"Dansk", "English", "Deutsch"};

int uart_transmit(char ch, FILE* stream)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = ch;

	return 0;
}

void pgm_read_block(void* dest, const void* src, uint16_t length) {
	uint8_t* d = (uint8_t*)dest;
	const uint8_t* s = (const uint8_t*)src;

    while (length--)
		*d++ = pgm_read_byte(s++);
}

ISR(TIMER1_COMPA_vect)
{
	int flashprom_languages_length = sizeof(flashprom_languages) / sizeof(flashprom_languages[0]);
	for (int i = 0; i < flashprom_languages_length; i++) {
		int flashprom_language_size = sizeof(flashprom_languages[i]);
		char flashprom_str[flashprom_language_size];
		pgm_read_block(&flashprom_str, &flashprom_languages[i], flashprom_language_size);

		printf("Current language: %s\r\n", flashprom_str);
	}
}

static FILE uart_output = FDEV_SETUP_STREAM(uart_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void) {
    UBRR0 = 103;  // 9600 baud @ 16 MHz
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);  // Enable RX + + TX + RX interrupt
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data

    TCCR1B |= (1 << WGM12); // Set CTC mode: WGM12 = 1
    TCCR1B |= (1 << CS12) | (1 << CS10); // Set prescaler to 1024
	// Calculate OCR1A for 1-second interval
    // F_CPU = 16 MHz, prescaler = 1024
    // Timer counts = 16,000,000 / 1024 = 15625 counts per second
	OCR1A = 15625; // 0..15624 = 15625 counts
    TIMSK1 |= (1 << OCIE1A); // Enable Timer1 Compare A Match Interrupt

    sei();	// Enable global interrupts

	stdout = &uart_output;

    while (1) {}

    return 0;
}

