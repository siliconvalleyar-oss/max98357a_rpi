# API Reference

## Device::Device_t

Main application class.

### Methods
- `void run()`: Starts the main application loop.
- `void showVersion() const`: Displays the application version.

## Device::Sound::Max98357A

Audio output driver for MAX98357A I2S amplifier.

### Methods
- `bool init(uint32_t sample_rate, uint8_t channels)`: Initializes ALSA PCM.
- `bool play(const int16_t* buffer, size_t frames)`: Plays audio buffer.
- `void close()`: Closes PCM device.
