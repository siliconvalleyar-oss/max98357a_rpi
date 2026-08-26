#include "MPG123Decoder.hpp"
#include <iostream>
#include <vector>

namespace Device {
namespace Sound {

MPG123Decoder::MPG123Decoder()
    : mpg_handle(nullptr), pcm_handle(nullptr), playing(false),
      sample_rate_(44100), channels_(1) {
    mpg123_init();
}

MPG123Decoder::~MPG123Decoder() {
    stop();
    mpg123_delete(mpg_handle);
    mpg123_exit();
}

bool MPG123Decoder::open(const std::string& filepath) {
    if (mpg_handle) {
        mpg123_close(mpg_handle);
        mpg123_delete(mpg_handle);
        mpg_handle = nullptr;
    }

    mpg_handle = mpg123_new(nullptr, nullptr);
    if (!mpg_handle) {
        std::cerr << "[MPG123] Error creando handle" << std::endl;
        return false;
    }

    if (mpg123_open(mpg_handle, filepath.c_str()) != MPG123_OK) {
        std::cerr << "[MPG123] Error abriendo: " << filepath << std::endl;
        return false;
    }

    long rate;
    int channels, encoding;
    if (mpg123_getformat(mpg_handle, &rate, &channels, &encoding) != MPG123_OK) {
        std::cerr << "[MPG123] Error obteniendo formato" << std::endl;
        return false;
    }

    sample_rate_ = rate;
    channels_ = channels;

    std::cout << "[MPG123] " << filepath
              << " (" << rate << "Hz, " << channels << "ch)" << std::endl;
    return true;
}

bool MPG123Decoder::initAlsa(long rate, int channels) {
    closeAlsa();

    int err = snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "[ALSA] Error abriendo PCM: " << snd_strerror(err) << std::endl;
        return false;
    }

    snd_pcm_hw_params_t* hw_params;
    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(pcm_handle, hw_params);
    snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, hw_params, channels);

    unsigned int r = rate;
    snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &r, 0);
    snd_pcm_hw_params(pcm_handle, hw_params);

    return true;
}

void MPG123Decoder::closeAlsa() {
    if (pcm_handle) {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
        pcm_handle = nullptr;
    }
}

void MPG123Decoder::decodeAndPlay() {
    size_t buffer_size = mpg123_outblock(mpg_handle);
    std::vector<unsigned char> buffer(buffer_size);
    size_t done;

    while (playing && mpg123_read(mpg_handle, buffer.data(), buffer_size, &done) == MPG123_OK) {
        if (done == 0) break;

        snd_pcm_sframes_t frames = done / (2 * channels_);
        snd_pcm_sframes_t written = snd_pcm_writei(pcm_handle, buffer.data(), frames);

        if (written < 0) {
            written = snd_pcm_recover(pcm_handle, written, 0);
            snd_pcm_writei(pcm_handle, buffer.data(), frames);
        }
    }
}

bool MPG123Decoder::play() {
    if (!mpg_handle) return false;

    if (!initAlsa(sample_rate_, channels_)) return false;

    playing = true;
    decodeAndPlay();
    playing = false;

    closeAlsa();
    return true;
}

void MPG123Decoder::stop() {
    playing = false;
    closeAlsa();
}

bool MPG123Decoder::isPlaying() const { return playing; }
long MPG123Decoder::sampleRate() const { return sample_rate_; }
int MPG123Decoder::channels() const { return channels_; }

} // namespace Sound
} // namespace Device
