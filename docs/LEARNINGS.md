# Learnings

- bcm2835 library requires root privileges for I2C/SPI.
- MAX98357A works via ALSA PCM, not direct I2S control from userspace.
- SSD1306 I2C address is usually 0x3C.
- Always check return values of hardware init functions.
