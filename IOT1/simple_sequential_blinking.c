#include <avr/io.h>
#include <util/delay.h>

#define RED_LED PB3 
#define YELLOW_LED PB2
#define GREEN_LED PB1

int main(void) {
	DDRB = 0xFF;

	int leds[2] = {RED_LED, YELLOW_LED};
	int leds_length = sizeof(leds) / sizeof(leds[0]);
	while (1) {
		for (int i = 0; i < leds_length; i++) {
			PORTB ^= (1 << leds[i]);
			_delay_ms(1000);
			PORTB ^= (1 << leds[i]);
		}
	}

    return 0;
}
