# FRDM-MCXA153 Puzzle Box - Hardware Connections Guide

## Board Overview
- **Target**: FRDM-MCXA153 Development Board
- **MCU**: NXP MCX A153 (Arm Cortex-M33)
- **Package**: 64-pin LQFP
- **Voltage**: 3.3V (NOT 5V tolerant)

---

## Connector Pinout Reference

### J2 & J3 - Arduino-Style Headers (GPIO)
```
J2 (Left header, top-to-bottom):
Pin 1  --> PC2   (RGB_LED3_B)
Pin 2  --> PC1   (RGB_LED3_G)
Pin 3  --> PC0   (RGB_LED3_R)
Pin 4  --> PD5   (RGB_LED2_B)
Pin 5  --> PD4   (RGB_LED2_G)
Pin 6  --> PD3   (RGB_LED2_R)
Pin 7  --> PD2   (RGB_LED1_B)
Pin 8  --> PD1   (RGB_LED1_G)
Pin 9  --> PD0   (RGB_LED1_R)
Pin 10 --> GND

J3 (Right header, top-to-bottom):
Pin 1  --> PC3   (RGB_LED4_R)
Pin 2  --> PC4   (RGB_LED4_G)
Pin 3  --> PC5   (RGB_LED4_B)
Pin 4  --> PE0   (RGB_LED5_R)
Pin 5  --> PE1   (RGB_LED5_G)
Pin 6  --> PE2   (RGB_LED5_B)
Pin 7  --> PB1   (SWITCH_POWER)
Pin 8  --> PB2   (SWITCH_RESET)
Pin 9  --> GND
Pin 10 --> VCC_3V3
```

### J4 - FRDM Header (Specialized I/O)
```
Pin 12 --> PB0   (LED_ERROR)
Pin 14 --> PE3   (LED_POWER)
Pin 16 --> PD6   (BUZZER)
GND    --> Ground pins
VCC    --> 3.3V power pins
```

### J5 - Pmod/Expansion Header (Keypad Matrix)
```
Pin 1  --> PA0   (KEYPAD_ROW1)
Pin 2  --> PA1   (KEYPAD_ROW2)
Pin 3  --> PA2   (KEYPAD_ROW3)
Pin 4  --> PA3   (KEYPAD_ROW4)
Pin 5  --> PA4   (KEYPAD_COL1)
Pin 6  --> PA5   (KEYPAD_COL2)
Pin 7  --> PA12  (KEYPAD_COL3)
Pin 8  --> PA13  (KEYPAD_COL4)
Pin 9  --> GND
Pin 10 --> VCC_3V3
```

### I2C & UART (Built-in Arduino-compatible pins on J2/J3)
```
OLED I2C Connections:
  SDA (Data)  --> PTC7  (J2 Arduino header, labeled D18/SDA)
  SCL (Clock) --> PTC6  (J2 Arduino header, labeled D19/SCL)

Bluetooth UART Connections:
  TX --> PTA3  (J4 header area)
  RX --> PTA2  (J4 header area)
```

---

## Hardware Components & Wiring

### 1. RGB LEDs (5x for Puzzle Indicators)
**Purpose**: Visual status for each puzzle (Green=solved, Yellow=waiting, Red=wrong)

**LED 1 (Puzzle 1)**
```
LED_R1 (cathode) --> PD0 (J2 Pin 9) via 220Ω resistor
LED_G1 (cathode) --> PD1 (J2 Pin 8) via 220Ω resistor
LED_B1 (cathode) --> PD2 (J2 Pin 7) via 220Ω resistor
LED_Anode (common) --> 3.3V power
```

**LED 2 (Puzzle 2)** - Same pattern with PD3/PD4/PD5
**LED 3 (Puzzle 3)** - Same pattern with PC0/PC1/PC2
**LED 4 (Puzzle 4)** - Same pattern with PC3/PC4/PC5
**LED 5 (Puzzle 5)** - Same pattern with PE0/PE1/PE2

### 2. Status LEDs (2x)
**LED_POWER (PE3)** - Green LED, on when system active
```
Cathode --> PE3 (J4 Pin 14) via 220Ω resistor
Anode   --> 3.3V power
```

**LED_ERROR (PB0)** - Red LED, on when error occurs
```
Cathode --> PB0 (J4 Pin 12) via 220Ω resistor
Anode   --> 3.3V power
```

### 3. Buzzer (PD6)
**Purpose**: Audio feedback for signals, morse code, success/error sounds

```
Buzzer+ --> PD6 (J4 Pin 16) via 220Ω resistor
Buzzer- --> GND
```

### 4. Push Buttons (2x)
**Purpose**: User control - power on/off, reset puzzle

**SWITCH_POWER (PB1)**
```
Terminal 1 --> PB1 (J3 Pin 7)
Terminal 2 --> GND
Note: Uses internal pullup resistor
```

**SWITCH_RESET (PB2)**
```
Terminal 1 --> PB2 (J3 Pin 8)
Terminal 2 --> GND
Note: Uses internal pullup resistor
```

### 5. 4x4 Keypad Matrix
**Purpose**: 16-button input for puzzle solving

**Row Pins (Output from MCU)**
```
Row 1 --> PA0 (J5 Pin 1)
Row 2 --> PA1 (J5 Pin 2)
Row 3 --> PA2 (J5 Pin 3)
Row 4 --> PA3 (J5 Pin 4)
```

**Column Pins (Input to MCU, use internal pullup)**
```
Col 1 --> PA4  (J5 Pin 5)
Col 2 --> PA5  (J5 Pin 6)
Col 3 --> PA12 (J5 Pin 7)
Col 4 --> PA13 (J5 Pin 8)
```

**Standard 4x4 Keypad Module**
```
Module Pins:
  Row 1 --> Keypad matrix pin (connect to PA0)
  Row 2 --> Keypad matrix pin (connect to PA1)
  Row 3 --> Keypad matrix pin (connect to PA2)
  Row 4 --> Keypad matrix pin (connect to PA3)
  Col 1 --> Keypad matrix pin (connect to PA4)
  Col 2 --> Keypad matrix pin (connect to PA5)
  Col 3 --> Keypad matrix pin (connect to PA12)
  Col 4 --> Keypad matrix pin (connect to PA13)
  GND   --> J5 Pin 9 (Ground)
```

### 6. OLED Display (I2C - SSD1306)
**Purpose**: Display puzzle status, instructions, countdown

**Pin Connections**
```
OLED Module:
  VCC   --> 3.3V (J2/J4 power pins)
  GND   --> Ground
  SDA   --> PTC7 (J2 Arduino SDA pin)
  SCL   --> PTC6 (J2 Arduino SCL pin)
```

**I2C Details**
- Module: LPI2C0
- Frequency: 100 kHz (Standard I2C)
- Address: 0x3C or 0x3D (verify with I2C scanner)
- Pull-ups: Usually built-in on OLED module

### 7. Bluetooth Module (HC-05/HC-06)
**Purpose**: Wireless puzzle signal input

**Connections & Voltage Considerations**
```
HC-05 Module:
  VCC   --> 5V power supply (use separate regulator if available)
           OR 3.3V if module supports it
  GND   --> Common ground
  TX    --> PTA2 (LPUART0 RX) - needs voltage divider!
  RX    --> PTA3 (LPUART0 TX) - 3.3V signal acceptable
  KEY   --> 3.3V (optional, for AT command mode)
```

**Critical: Voltage Divider for Bluetooth RX Signal**
```
HC-05 TX (5V) ---[10kΩ]---+--- PTA2 (MCU RX)
                           |
                         [20kΩ] (to GND)
                           |
                          GND

This reduces 5V signal to ~3.3V for safe input
```

**Alternative: Use Dedicated Level Shifter Module**
- Easier and more reliable than voltage divider
- Popular options: TXS0102, TXB0104, or similar I2C level shifter

---

## Power Distribution

### 3.3V Power Rail
- All GPIO outputs (LEDs, buzzer)
- All GPIO inputs (buttons, keypad, OLED, Bluetooth RX)
- OLED display power
- Bluetooth module (if 3.3V version) or via level shifter

### 5V Power Rail (if available on development board)
- Bluetooth module (HC-05 typically 5V)
- Status indicator LEDs (optional, higher brightness)

### Ground
- Common GND for all peripherals
- Multiple GND pins available on all headers

---

## MCUXpresso SDK Configuration

### GPIO Setup
```c
// Enable GPIO clocks for all needed ports
CLOCK_EnableClock(kCLOCK_PortA);
CLOCK_EnableClock(kCLOCK_PortB);
CLOCK_EnableClock(kCLOCK_PortC);
CLOCK_EnableClock(kCLOCK_PortD);
CLOCK_EnableClock(kCLOCK_PortE);

// Configure LED GPIO pins as outputs (digital I/O)
gpio_pin_config_t led_config = { kGPIO_DigitalOutput, 0 };
GPIO_PinInit(GPIOD, 0, &led_config);  // PD0 for LED1_R
// ... repeat for all LED pins

// Configure button pins as inputs with pullup
gpio_pin_config_t button_config = { kGPIO_DigitalInput, 0 };
GPIO_PinInit(GPIOB, 1, &button_config);  // PB1 for POWER button
// ... repeat for RESET button
```

### UART Configuration (Bluetooth)
```c
// LPUART0: 9600 baud, 8-bit, 1 stop bit, no parity
lpuart_config_t uart_config;
LPUART_GetDefaultConfig(&uart_config);
uart_config.baudRate_Bps = 9600;
LPUART_Init(LPUART0, &uart_config, CLOCK_GetFreq(kCLOCK_CoreSysClk));
```

### I2C Configuration (OLED)
```c
// LPI2C0: 100 kHz standard mode
lpi2c_master_config_t i2c_config;
LPI2C_MasterGetDefaultConfig(&i2c_config);
i2c_config.baudRate_Hz = 100000;
LPI2C_MasterInit(LPI2C0, &i2c_config, CLOCK_GetFreq(kCLOCK_BusClk));
```

---

## Testing Checklist

- [ ] All LEDs light up correctly when GPIO set high
- [ ] Buttons register presses when connected to GND
- [ ] Buzzer makes sound when GPIO driven high
- [ ] Keypad scanning detects button presses
- [ ] OLED displays text via I2C (address 0x3C or 0x3D)
- [ ] Bluetooth module receives signals at 9600 baud
- [ ] Voltage divider or level shifter working for Bluetooth RX

---

## Troubleshooting

### LED not lighting
- Check polarity (cathode to GPIO, anode to 3.3V)
- Verify GPIO is set HIGH (1)
- Test with multimeter for voltage on LED pins

### Button not registering
- Check button connected to both GPIO pin AND GND
- Verify internal pullup is enabled in GPIO configuration
- Test with GPIO input reading

### Keypad not working
- Check all 8 wire connections (4 rows + 4 columns)
- Verify row pins set as output, column pins as input
- Use logic analyzer or oscilloscope to debug scanning

### OLED not displaying
- Verify I2C address with Arduino i2c-scanner sketch
- Check SDA/SCL voltages with multimeter
- Ensure pullup resistors present (usually built-in)

### Bluetooth not communicating
- Check HC-05 LED blinks for pairing status
- Verify UART baud rate set to 9600
- Confirm voltage divider on RX line
- Try AT command mode (KEY pin to 3.3V, send "AT\r\n")

---

## Quick Reference - Pin Summary

| Component | Pin(s) | Header | Notes |
|-----------|--------|--------|-------|
| RGB_LED1 | PD0-PD2 | J2 (9,8,7) | Puzzle 1 |
| RGB_LED2 | PD3-PD5 | J2 (6,5,4) | Puzzle 2 |
| RGB_LED3 | PC0-PC2 | J2 (3,2,1) | Puzzle 3 |
| RGB_LED4 | PC3-PC5 | J3 (1,2,3) | Puzzle 4 |
| RGB_LED5 | PE0-PE2 | J3 (4,5,6) | Puzzle 5 |
| LED_POWER | PE3 | J4 (14) | Status |
| LED_ERROR | PB0 | J4 (12) | Status |
| BUZZER | PD6 | J4 (16) | PWM capable |
| BUTTON_POWER | PB1 | J3 (7) | Pullup |
| BUTTON_RESET | PB2 | J3 (8) | Pullup |
| KEYPAD_ROWS | PA0-PA3 | J5 (1-4) | Outputs |
| KEYPAD_COLS | PA4-PA5, PA12-PA13 | J5 (5-8) | Inputs |
| OLED_SDA | PTC7 | J2 | I2C |
| OLED_SCL | PTC6 | J2 | I2C |
| BT_TX | PTA3 | J4 area | LPUART0 |
| BT_RX | PTA2 | J4 area | LPUART0 |
