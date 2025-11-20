MCU = atmega328p
F_CPU = 16000000UL
CC = avr-gcc
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -I lib
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
PROGRAMMER = xplainedmini
PORT = usb

BUILD_DIR = build
TARGET_ELF = $(BUILD_DIR)/main.elf
TARGET_HEX = $(BUILD_DIR)/main.hex
OBJ = $(BUILD_DIR)/main.o $(BUILD_DIR)/ssd1306.o $(BUILD_DIR)/twi.o

all: $(BUILD_DIR) $(TARGET_HEX)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile ELF
$(TARGET_ELF): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: lib/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile object files into build folder
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Convert ELF to HEX
$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Flash to board
flash: $(TARGET_HEX)
	$(AVRDUDE) -F -v -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -U flash:w:$(TARGET_HEX)

clean:
	rm -rf $(BUILD_DIR)
