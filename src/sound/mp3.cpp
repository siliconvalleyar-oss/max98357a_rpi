#include "mp3.hpp"
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace Device {
namespace Sound {

MP3Player::MP3Player() : current(-1) {}
MP3Player::~MP3Player() { stop(); }

bool MP3Player::loadDir(const std::string& dirpath) {
    tracks.clear();
    current = -1;

    if (!fs::exists(dirpath) || !fs::is_directory(dirpath)) {
        std::cerr << "[MP3] Directorio no encontrado: " << dirpath << std::endl;
        return false;
    }

    for (const auto& entry : fs::directory_iterator(dirpath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".mp3") {
                tracks.push_back(entry.path().string());
            }
        }
    }

    std::sort(tracks.begin(), tracks.end());

    std::cout << "[MP3] " << tracks.size() << " archivos encontrados en " << dirpath << std::endl;
    for (size_t i = 0; i < tracks.size(); i++) {
        std::cout << "  " << i + 1 << ". " << fs::path(tracks[i]).filename().string() << std::endl;
    }

    return !tracks.empty();
}

bool MP3Player::load(const std::string& filepath) {
    return decoder.open(filepath);
}

bool MP3Player::play() {
    if (tracks.empty()) {
        std::cerr << "[MP3] No hay pistas cargadas" << std::endl;
        return false;
    }

    if (current < 0) current = 0;

    if (!decoder.open(tracks[current])) return false;

    std::cout << "[MP3] Reproduciendo: " << fs::path(tracks[current]).filename().string() << std::endl;
    return decoder.play();
}

void MP3Player::stop() {
    decoder.stop();
}

bool MP3Player::next() {
    if (tracks.empty()) return false;
    current = (current + 1) % tracks.size();
    return play();
}

bool MP3Player::prev() {
    if (tracks.empty()) return false;
    current = (current - 1 + tracks.size()) % tracks.size();
    return play();
}

bool MP3Player::isPlaying() const { return decoder.isPlaying(); }
const std::vector<std::string>& MP3Player::getTracks() const { return tracks; }
int MP3Player::currentIndex() const { return current; }

void MP3Player::setVolume(float v) { decoder.setVolume(v); }

} // namespace Sound
} // namespace Device
