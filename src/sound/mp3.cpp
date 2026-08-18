#include "mp3.hpp"

namespace Device {
namespace Sound {

MP3Player::MP3Player() : playing(false) {}
MP3Player::~MP3Player() {}

bool MP3Player::load(const std::string& filepath) { return false; }
bool MP3Player::play() { playing = true; return true; }
void MP3Player::stop() { playing = false; }
bool MP3Player::isPlaying() const { return playing; }

} // namespace Sound
} // namespace Device
