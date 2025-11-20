#include <avr/interrupt.h>

#include <stdio.h>

#include <ssd1306.h>

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

	uint8_t display_status = SSD1306_Init(SSD1306_ADDR);
	if (SSD1306_SUCCESS != display_status) {
		printf("Display NOT properly initialized!\r\n");
	} else {
		printf("Display has been initialized.\r\n");
	}

    while (1) {
		display_status = SSD1306_DrawString("h4pd011125 styrer", NORMAL);
		if (SSD1306_SUCCESS != display_status) {
			printf("Error while trying to draw string to screen - %d\r\n", 
					display_status);
		}
	}

    return 0;
}

