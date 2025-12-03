#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define DHT_PIN_PORT    PORTD 
#define DHT_PIN_DDR     DDRD
#define DHT_PIN_INPUT   PIND
#define DHT_DATA_PIN    PD2

struct Packet {
  uint8_t rh_integral;
  uint8_t rh_decimal;
  uint8_t t_integral;
  uint8_t t_decimal;
  uint8_t checksum;
};

uint8_t dht22_read_bit(void) {
  uint16_t timeout = 0;
  uint8_t bit = 0;
    
  while (!(DHT_PIN_INPUT & (1 << DHT_DATA_PIN))) {
	timeout++;
	if (timeout > 200) return 2; 
  }

  _delay_us(40);
    
  bit = 0;
  if (DHT_PIN_INPUT & (1 << DHT_DATA_PIN)) {
	bit = 1;
  }

  timeout = 0;
  while (DHT_PIN_INPUT & (1 << DHT_DATA_PIN)) {
	timeout++;
	if (timeout > 200) return 2; 
  }

  return bit;
}

int uart_transmit(char ch, FILE* stream) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = ch;
  
  return 0;
}

static FILE uart_output = FDEV_SETUP_STREAM(uart_transmit, NULL, _FDEV_SETUP_WRITE);

int main(void) {
  uint8_t i, j = 0;
  uint8_t data[5] = {0};
  
  /* Set baud rate */
  const uint8_t ubrr = ((F_CPU/16/9600)-1);
  UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)ubrr;
  /* Enable receiver and transmitter */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  /* Set frame format: 8 data, 2 stop bit */
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);

  stdout = &uart_output;

  while (1) {
	DHT_PIN_DDR |= (1 << DHT_DATA_PIN);
	DHT_PIN_PORT &= ~(1 << DHT_DATA_PIN);
	_delay_ms(5); 
    
	DHT_PIN_PORT |= (1 << DHT_DATA_PIN);
	DHT_PIN_DDR &= ~(1 << DHT_DATA_PIN);
	_delay_us(40);

	if (DHT_PIN_INPUT & (1 << DHT_DATA_PIN)) return 1;
	_delay_us(80);
	if (!(DHT_PIN_INPUT & (1 << DHT_DATA_PIN))) return 1;
	_delay_us(80);

	for (j = 0; j < 5; j++) {
	  for (i = 0; i < 8; i++) {
		uint8_t bit = dht22_read_bit();
		if (bit == 2) return 2;

		data[j] <<= 1;
		if (bit == 1) {
		  data[j] |= 1;
		}
	  }
	}

	if (data[4] == (data[0] + data[1] + data[2] + data[3])) {
	  struct Packet* packet = (struct Packet*) data;	
	  printf("RHintegral = %d, RHdecimal = %d, Tintegral = %d, Tdecimal = %d, checksum = %d.\r\n",
			 packet->rh_integral, packet->rh_decimal, packet->t_integral,
			 packet->t_decimal, packet->checksum);
	}

	_delay_ms(2500); 
  }
}
