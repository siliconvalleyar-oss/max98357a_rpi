#ifndef MAX98357A_HPP
#define MAX98357A_HPP

#include <cstdint>
#include <string>
#include <alsa/asoundlib.h>

namespace Device {
namespace Sound {

std::string max98357aDevice();

class Max98357A {
public:
    Max98357A();
    ~Max98357A();
    bool init(uint32_t sample_rate = 44100, uint8_t channels = 1);
    bool play(const int16_t* buffer, size_t frames);
    void close();
    void setVolume(float v);

private:
    snd_pcm_t* pcm_handle;
    uint32_t sample_rate_;
    uint8_t channels_;
    float volume_;
};

} // namespace Sound
} // namespace Device

#endif
