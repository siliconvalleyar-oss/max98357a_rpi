#include "Device_t.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <filesystem>
#include <bcm2835.h>
#include <oled/SSD1306_OLED.hpp>
#include <sound/max98357a.hpp>
#include <sound/mp3.hpp>

#ifndef VERSION
#define VERSION "0.1.0"
#endif

namespace fs = std::filesystem;

namespace Device {

Device_t::Device_t() : oled_initialized(false), mp3_dir("mp3") {}

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
    std::cout << "=== Dispositivo ===" << std::endl;
    std::cout << "Version: " << VERSION << std::endl;
    std::cout << "OLED: " << (oled_initialized ? "Conectado" : "No detectado (consola)") << std::endl;
    std::cout << "Audio: MAX98357A I2S" << std::endl;
    std::cout << "===================" << std::endl;
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

void Device_t::playMp3Folder() {
    Sound::MP3Player player;

    if (!player.loadDir(mp3_dir)) {
        std::cout << "[MP3] No se encontraron archivos en " << mp3_dir << std::endl;
        return;
    }

    for (int i = 0; i < player.getTracks().size(); i++) {
        std::cout << "[MP3] Reproduciendo pista " << i + 1 << "/" << player.getTracks().size() << std::endl;
        if (!player.play()) {
            std::cerr << "[MP3] Error reproduciendo pista" << std::endl;
        }
    }

    std::cout << "[MP3] Fin de la lista." << std::endl;
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
    playMp3Folder();

    std::cout << "Presiona Ctrl+C para salir." << std::endl;
    bcm2835_delay(5000);

    std::cout << "Finalizando..." << std::endl;
}

} // namespace Device
