#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PB5

int main(void)
{
    DDRB = 0xFF;

    while (1) {
        PORTB ^= (1 << LED_PIN);
        _delay_ms(500);
    }
}
