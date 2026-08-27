#include "max98357a.hpp"
#include <cstring>
#include <iostream>

namespace Device {
namespace Sound {

std::string max98357aDevice() {
    int card = -1;
    if (snd_card_next(&card) != 0) return "default";
    while (card >= 0) {
        char* name = nullptr;
        if (snd_card_get_name(card, &name) == 0) {
            std::string n(name);
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            free(name);
            if (n.find("max98357a") != std::string::npos) {
                return "plughw:" + std::to_string(card) + ",0";
            }
        }
        if (snd_card_next(&card) != 0) break;
    }
    return "default";
}

Max98357A::Max98357A() : pcm_handle(nullptr), sample_rate_(44100), channels_(1) {}

Max98357A::~Max98357A() { close(); }

bool Max98357A::init(uint32_t sample_rate, uint8_t channels) {
    sample_rate_ = sample_rate;
    channels_ = channels;

    int err = snd_pcm_open(&pcm_handle, max98357aDevice().c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Error abriendo PCM: " << snd_strerror(err) << std::endl;
        return false;
    }

    snd_pcm_hw_params_t* hw_params;
    snd_pcm_hw_params_alloca(&hw_params);
    err = snd_pcm_hw_params_any(pcm_handle, hw_params);
    if (err < 0) {
        std::cerr << "Error obteniendo parametros HW: " << snd_strerror(err) << std::endl;
        return false;
    }

    err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        std::cerr << "Error estableciendo acceso: " << snd_strerror(err) << std::endl;
        return false;
    }

    err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    if (err < 0) {
        std::cerr << "Error estableciendo formato: " << snd_strerror(err) << std::endl;
        return false;
    }

    err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, channels_);
    if (err < 0) {
        std::cerr << "Error estableciendo canales: " << snd_strerror(err) << std::endl;
        return false;
    }

    unsigned int rate = sample_rate_;
    err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rate, 0);
    if (err < 0) {
        std::cerr << "Error estableciendo tasa: " << snd_strerror(err) << std::endl;
        return false;
    }

    err = snd_pcm_hw_params(pcm_handle, hw_params);
    if (err < 0) {
        std::cerr << "Error aplicando parametros HW: " << snd_strerror(err) << std::endl;
        return false;
    }

    return true;
}

bool Max98357A::play(const int16_t* buffer, size_t frames) {
    if (!pcm_handle) return false;

    size_t offset = 0;
    while (offset < frames) {
        snd_pcm_sframes_t written =
            snd_pcm_writei(pcm_handle, buffer + offset, frames - offset);
        if (written < 0) {
            if (written == -EPIPE) {
                snd_pcm_prepare(pcm_handle);
                continue;
            }
            std::cerr << "Error escribiendo audio: " << snd_strerror(written) << std::endl;
            return false;
        }
        if (written == 0) break;
        offset += static_cast<size_t>(written);
    }
    return true;
}

void Max98357A::close() {
    if (pcm_handle) {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
        pcm_handle = nullptr;
    }
}

} // namespace Sound
} // namespace Device
