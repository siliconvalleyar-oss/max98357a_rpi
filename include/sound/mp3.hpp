#ifndef MP3_HPP
#define MP3_HPP

#include <string>
#include <cstdint>

namespace Device {
namespace Sound {

class MP3Player {
public:
    MP3Player();
    ~MP3Player();
    bool load(const std::string& filepath);
    bool play();
    void stop();
    bool isPlaying() const;

private:
    bool playing;
};

} // namespace Sound
} // namespace Device

#endif
