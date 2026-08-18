# Troubleshooting

## OLED not working
- Check I2C address (use `i2cdetect -y 1`).
- Ensure bcm2835 is initialized.
- Check wiring.

## Audio not working
- Ensure ALSA is configured for MAX98357A.
- Check I2S pins.
- Run `aplay /usr/share/sounds/alsa/Front_Left.wav`.
