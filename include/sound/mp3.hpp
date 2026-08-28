#ifndef MP3_HPP
#define MP3_HPP

#include <string>
#include <vector>
#include <cstdint>
#include "MPG123Decoder.hpp"

namespace Device {
namespace Sound {

class MP3Player {
public:
    MP3Player();
    ~MP3Player();

    bool loadDir(const std::string& dirpath);
    bool load(const std::string& filepath);
    bool play();
    void stop();
    bool next();
    bool prev();
    bool isPlaying() const;
    const std::vector<std::string>& getTracks() const;
    int currentIndex() const;
    void setVolume(float v);

private:
    std::vector<std::string> tracks;
    int current;
    MPG123Decoder decoder;
};

} // namespace Sound
} // namespace Device

#endif
