#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define DHT_PIN_PORT    PORTD 
#define DHT_PIN_DDR     DDRD
#define DHT_PIN_INPUT   PIND
#define DHT_DATA_PIN    PD2

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / 16 / BAUD) - 1) 

// --- Function Prototypes ---
void uart_init(void);
void uart_tx(char c);
void uart_puts(const char* s);
void print_hex_byte(char *buffer, uint8_t byte);
uint8_t dht22_read_bit(void);
uint8_t dht22_read_data(int16_t *temperature, int16_t *humidity);

// --- UART Implementation ---

void uart_init(void) {
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_tx(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uart_puts(const char* s) {
    while (*s) {
        uart_tx(*s++);
    }
}

/**
 * @brief Utility function to convert a byte to a two-character hex string.
 * @param buffer Pointer to a buffer (must be size 3) to store the result.
 * @param byte The byte to convert.
 */
void print_hex_byte(char *buffer, uint8_t byte) {
    const char hex_chars[] = "0123456789ABCDEF";
    buffer[0] = hex_chars[(byte >> 4) & 0x0F];
    buffer[1] = hex_chars[byte & 0x0F];
    buffer[2] = '\0';
}


// --- DHT22 Protocol Implementation ---

uint8_t dht22_read_bit(void) {
    uint16_t timeout = 0; 
    
    // 1. Wait for data line to go HIGH (Start of the measurement pulse, 28us or 70us long)
    while (!(DHT_PIN_INPUT & (1 << DHT_DATA_PIN))) {
        timeout++;
        if (timeout > 200) return 2; 
    }

    // 2. Wait for 40us. This is the discrimination point:
    //    - If the pulse was a 0 (approx 28us), the line will be LOW after this delay.
    //    - If the pulse was a 1 (approx 70us), the line will still be HIGH.
    _delay_us(40); // CRITICAL CHANGE: Set to 40us for pulse width discrimination.
    
    uint8_t bit = 0;
    if (DHT_PIN_INPUT & (1 << DHT_DATA_PIN)) {
        bit = 1;
    }

    // 3. Wait for the end of the pulse (to prepare for the next bit reading)
    timeout = 0;
    while (DHT_PIN_INPUT & (1 << DHT_DATA_PIN)) {
        timeout++;
        if (timeout > 200) return 2; 
    }

    return bit;
}

uint8_t dht22_read_data(int16_t *temperature, int16_t *humidity) {
    uint8_t bits[5] = {0};
    uint8_t i, j;
    
    // --- 1. Send Start Signal ---
    DHT_PIN_DDR |= (1 << DHT_DATA_PIN);
    DHT_PIN_PORT &= ~(1 << DHT_DATA_PIN);
    _delay_ms(5); 
    
    // --- 2. Release bus and switch to INPUT ---
    DHT_PIN_PORT |= (1 << DHT_DATA_PIN);
    DHT_PIN_DDR &= ~(1 << DHT_DATA_PIN);
    _delay_us(40);

    // --- 3. Check DHT Response ---
    if (DHT_PIN_INPUT & (1 << DHT_DATA_PIN)) return 1;
    _delay_us(80);
    if (!(DHT_PIN_INPUT & (1 << DHT_DATA_PIN))) return 1;
    _delay_us(80);

    // --- 4. Read 40 bits of data (5 bytes) ---
    for (j = 0; j < 5; j++) {
        for (i = 0; i < 8; i++) {
            uint8_t bit = dht22_read_bit();
            if (bit == 2) return 2;

            bits[j] <<= 1;
            if (bit == 1) {
                bits[j] |= 1;
            }
        }
    }

    // --- 5. Checksum verification ---
    uint8_t calculated_checksum = bits[0] + bits[1] + bits[2] + bits[3];
    uint8_t received_checksum = bits[4];
    
    if (calculated_checksum == received_checksum) {
        
        uint16_t raw_humidity = bits[0] << 8 | bits[1];
        uint16_t raw_temperature = bits[2] << 8 | bits[3];

        *humidity = (int16_t)raw_humidity;

        if (raw_temperature & 0x8000) {
            *temperature = -((int16_t)(raw_temperature & 0x7FFF));
        } else {
            *temperature = (int16_t)raw_temperature;
        }

        return 0; // SUCCESS!
    }

    // --- 6. Debugging: Print Raw Data ONLY on Checksum Failure (Error 3) ---
    char byte_str[3];
    uart_puts("Raw Data: ");
    
    print_hex_byte(byte_str, bits[0]); uart_puts(byte_str); uart_puts(" "); // Hum High
    print_hex_byte(byte_str, bits[1]); uart_puts(byte_str); uart_puts(" "); // Hum Low
    print_hex_byte(byte_str, bits[2]); uart_puts(byte_str); uart_puts(" "); // Temp High
    print_hex_byte(byte_str, bits[3]); uart_puts(byte_str); uart_puts(" | "); // Temp Low
    print_hex_byte(byte_str, received_checksum); uart_puts(byte_str); // Checksum Received
    uart_puts("\r\n");

    uart_puts("CS (Calc: ");
    print_hex_byte(byte_str, calculated_checksum);
    uart_puts(byte_str);
    uart_puts(" | Recv: ");
    print_hex_byte(byte_str, received_checksum);
    uart_puts(byte_str);
    uart_puts(") - Checksum Failed\r\n\r\n");
    
    return 3; // FAILURE
}

// --- Main Program ---

int main(void) {
    uart_init();
    uart_puts("DHT22 Debug Logger Initialized (16MHz).\r\n");

    int16_t temperature_raw, humidity_raw;
    char buffer[64];

    _delay_ms(2000); 

    while (1) {
        uint8_t result = dht22_read_data(&temperature_raw, &humidity_raw);

        if (result == 0) {
            // Success printing 
            sprintf(buffer, "Humidity: %d.%d%%, ", 
                    humidity_raw / 10, humidity_raw % 10);
            uart_puts(buffer);
            
            int16_t abs_temp = (temperature_raw >= 0) ? temperature_raw : -temperature_raw;
            sprintf(buffer, "Temperature: %s%d.%d C\r\n\r\n", 
                    (temperature_raw < 0) ? "-" : "", 
                    abs_temp / 10, abs_temp % 10);
            uart_puts(buffer);
            
        } else {
            // Error 1 and Error 2 print explicitly here. Error 3 prints detailed info inside dht22_read_data.
            if (result == 1) uart_puts("DHT22 Read Error: 1 (No Response)\r\n\r\n");
            if (result == 2) uart_puts("DHT22 Read Error: 2 (Timeout)\r\n\r\n");
            // Note: If result == 3, the detailed debug output was already printed.
        }

        _delay_ms(2500); 
    }
}