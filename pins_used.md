# FRDM-MCXA153 - All Pins Used

This file lists all the pins from the FRDM-MCXA153 board that are connected to external components.

## Power and Ground
```
7 --> 3V3 (Power)
8 --> GND (Ground)
```

## Port P1 Pins
```
P1_4  --> Bluetooth HM-10 TX
P1_5  --> Bluetooth HM-10 RX
P1_6  --> Pushbutton (3) Pin 3/Pin 4
P1_7  --> Pushbutton (2) Pin 3/Pin 4
P1_8  --> Pushbutton (1) Pin 3/Pin 4
P1_9  --> Pushbutton (no suffix) Pin 3/Pin 4
P1_10 --> RGB LED R via 220Ω resistor
P1_11 --> RGB LED G via 220Ω resistor
P1_13 --> RGB LED R via 220Ω resistor
```

## Port P2 Pins
```
P2_0  --> Keypad Col 1
P2_1  --> Keypad Col 2
P2_2  --> Keypad Col 3
P2_4  --> Keypad Row 1
P2_5  --> Keypad Row 2 (INT)
P2_6  --> Keypad Row 3
P2_7  --> Keypad Row 4
P2_12 --> RGB LED R via 220Ω resistor
P2_13 --> RGB LED G via 220Ω resistor
```

## Port P3 Pins
```
P3_0  --> RGB LED G via 220Ω resistor
P3_6  --> Green LED anode via 220Ω resistor
P3_7  --> Blue LED anode via 220Ω resistor
P3_8  --> Yellow LED anode via 330Ω resistor
P3_9  --> Red LED anode via 330Ω resistor
P3_10 --> RGB LED R via 220Ω resistor
P3_11 --> RGB LED G via 220Ω resistor
P3_13 --> RGB LED R via 220Ω resistor
P3_14 --> RGB LED G via 220Ω resistor
P3_27 --> OLED SCL
P3_28 --> OLED SDA
P3_30 --> Buzzer VCC
```

## Summary
- **Total pins used**: 27 (excluding power/ground)
- **Ports used**: P1 (7 pins), P2 (9 pins), P3 (12 pins)
- **Components connected**: Bluetooth HM-10, OLED 128x32, 4x3 Keypad, Buzzer, 4 Single LEDs, 5 RGB LEDs, 4 Pushbuttons