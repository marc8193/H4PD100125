#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char flashprom_timer_interrupt_str[] PROGMEM = "Flashprom str for timer interrupt";
const char flashprom_button_interrupt_str[] PROGMEM = "Flashprom str for button interrupt";

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
	int flashprom_str_length = sizeof(flashprom_timer_interrupt_str);
	char flashprom_str[flashprom_str_length];
	pgm_read_block(&flashprom_str, &flashprom_timer_interrupt_str, flashprom_str_length);

	printf("%s\r\n", flashprom_str);
}

ISR(PCINT1_vect)
{
	int flashprom_str_length = sizeof(flashprom_button_interrupt_str);
	char flashprom_str[flashprom_str_length];
	pgm_read_block(&flashprom_str, &flashprom_button_interrupt_str, flashprom_str_length);

	printf("%s\r\n", flashprom_str);
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

    // Set PC5 as input
    DDRC &= ~(1 << PC5);
    PORTC |= (1 << PC5);  // Enable pull-up resistor

    // Enable Pin Change Interrupt for PCINT13 (PC5)
    PCICR |= (1 << PCIE1);     // Enable PCINT[14:8] group (Port C)
    PCMSK1 |= (1 << PCINT13);  // Enable interrupt for PC5

    sei();	// Enable global interrupts

	stdout = &uart_output;

    while (1) {}

    return 0;
}

