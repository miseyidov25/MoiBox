# FRDM-MCXA153 Puzzle Box - Hardware Connections Guide

## Board Overview
- **Target**: FRDM-MCXA153 Development Board
- **MCU**: NXP MCX A153 (Arm Cortex-M33)
- **Package**: 64-pin LQFP
- **Voltage**: 3.3V (NOT 5V tolerant)

---

## Power and Ground Connections

```
7 3V3 --> HM-10 VCC, OLED 128x32 VCC
8 GND --> HM-10 GND, OLED 128x32 GND, Buzzer GND, RGB LEDs (all 5) common "-", single LEDs (Red/Green/Blue/Yellow) cathodes, all pushbuttons Pin 1 and Pin 2
```

---

## Component Connections

### Bluetooth HM-10
```
4 P1_5 --> HM-10 RX
2 P1_4 --> HM-10 TX
```

### OLED 128x32 (I2C)
```
8 P3_28 SDA --> OLED SDA
6 P3_27 SCL --> OLED SCL
```

### 4x3 Keypad
```
A3 P2_0 --> Col 1
7 P2_1 --> Col 2
9 P2_2 --> Col 3
2 P2_4 --> Row 1
2 P2_5 INT --> Row 2
6 P2_6 --> Row 3
1 P2_7 --> Row 4
```

### Buzzer
```
1 P3_30 AN --> Buzzer VCC
```

### Single LEDs (via series resistors)
```
15 P3_6 --> Resistor (220 Ω) --> Green LED anode
13 P3_7 --> Resistor (220 Ω) --> Blue LED anode
11 P3_8 --> Resistor (330 Ω) --> Yellow LED anode
9 P3_9 --> Resistor (330 Ω) --> Red LED anode
```

### RGB LED (Common Cathode) (instance 9b06acf2)
```
7 P3_10 --> Resistor (220 Ω) --> R
5 P3_11 --> Resistor (220 Ω) --> G
```

### RGB LED (Common Cathode) (instance ca9fe375)
```
14 P3_13 --> Resistor (220 Ω) --> R
4 P3_14 --> Resistor (220 Ω) --> G
```

### RGB LED (Common Cathode) (instance 39b7c2eb)
```
12 P2_12 --> Resistor (220 Ω) --> R
8 P2_13 --> Resistor (220 Ω) --> G
```

### RGB LED (Common Cathode) (instance 7fa0cc45)
```
A0 P1_10 --> Resistor (220 Ω) --> R
9 P1_11 --> Resistor (220 Ω) --> G
```

### RGB LED (Common Cathode) (instance 93348254)
```
A2 P1_13 --> Resistor (220 Ω) --> R
8 P3_0 --> Resistor (220 Ω) --> G
```

### Pushbuttons
```
20 P1_9 --> Pushbutton (no suffix) Pin 3/Pin 4 (shorted together)
18 P1_8 --> Pushbutton (1) Pin 3/Pin 4 (shorted together)
1 P1_7 --> Pushbutton (2) Pin 3/Pin 4 (shorted together)
3 P1_6 --> Pushbutton (3) Pin 3/Pin 4 (shorted together)
```