# Hardware

## Components
- Raspberry Pi (any model with I2C and I2S)
- MAX98357A I2S Audio Amplifier
- SSD1306 OLED Display (128x64, I2C address 0x3C)
- Jumper wires, breadboard

## Connections
### I2C (OLED)
- SDA -> GPIO2 (SDA)
- SCL -> GPIO3 (SCL)
- VCC -> 3.3V
- GND -> GND

### I2S (MAX98357A)
- BCLK -> GPIO18
- LRC -> GPIO19
- DIN -> GPIO21
- GAIN -> No conectar (3dB por defecto)
- SD -> No conectar (habilitado por defecto)
- GND -> GND
- VIN -> 3.3V or 5V
