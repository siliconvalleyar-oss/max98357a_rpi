#include "Device_t.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <bcm2835.h>
#include <oled/SSD1306_OLED.hpp>
#include <sound/max98357a.hpp>

#ifndef VERSION
#define VERSION "0.1.0"
#endif

namespace Device {

Device_t::Device_t() : oled_initialized(false) {}

Device_t::~Device_t() {}

void Device_t::showVersion() const {
    std::cout << "App v" << VERSION << std::endl;
}

void Device_t::initBcm2835() {
    if (!bcm2835_init()) {
        std::cerr << "Error: No se pudo inicializar bcm2835. Ejecuta como root?" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "[bcm2835] Inicializado correctamente." << std::endl;
}

void Device_t::initOLED() {
    SSD1306 oled(128, 64);
    oled.OLEDbegin();
    oled.OLEDclearBuffer();
    oled.OLEDupdate();
    oled_initialized = true;
    std::cout << "[OLED] SSD1306 inicializado." << std::endl;
}

void Device_t::displayInfo() {
}

void Device_t::playTestTone() {
    Sound::Max98357A audio;
    if (!audio.init(44100, 1)) {
        std::cerr << "[Audio] Error inicializando MAX98357A" << std::endl;
        return;
    }

    const int sample_rate = 44100;
    const int duration = 2;
    const int num_samples = sample_rate * duration;
    std::vector<int16_t> buffer(num_samples);
    for (int i = 0; i < num_samples; ++i) {
        buffer[i] = static_cast<int16_t>(32767 * std::sin(2 * M_PI * 1000 * i / sample_rate));
    }

    audio.play(buffer.data(), num_samples);
    std::cout << "[Audio] Tono de prueba reproducido." << std::endl;
}

void Device_t::cleanup() {
    bcm2835_i2c_end();
    bcm2835_spi_end();
    bcm2835_close();
}

void Device_t::run() {
    std::cout << "=== App v" << VERSION << " ===" << std::endl;
    std::cout << "Iniciando dispositivo..." << std::endl;

    initBcm2835();
    initOLED();
    displayInfo();
    playTestTone();

    std::cout << "Presiona Ctrl+C para salir." << std::endl;
    bcm2835_delay(5000);

    std::cout << "Finalizando..." << std::endl;
}

} // namespace Device
