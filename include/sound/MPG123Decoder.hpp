#ifndef MPG123DECODER_HPP
#define MPG123DECODER_HPP

#include <string>
#include <cstdint>
#include <atomic>
#include <mpg123.h>
#include <alsa/asoundlib.h>

namespace Device {
namespace Sound {

class MPG123Decoder {
public:
    MPG123Decoder();
    ~MPG123Decoder();

    bool open(const std::string& filepath);
    bool play();
    void stop();
    bool isPlaying() const;
    long sampleRate() const;
    int channels() const;

private:
    bool initAlsa(long rate, int channels);
    void closeAlsa();
    void decodeAndPlay();

    mpg123_handle* mpg_handle;
    snd_pcm_t* pcm_handle;
    std::atomic<bool> playing;
    long sample_rate_;
    int channels_;
};

} // namespace Sound
} // namespace Device

#endif
