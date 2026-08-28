#include <memory>
#include <iostream>
#include <string>

#include "Device_t.hpp"

int main(int argc, char* argv[]) {
    float volume = -1.0f;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--version") {
            std::cout << "App v" << VERSION << std::endl;
            return 0;
        } else if (arg == "--volume" && i + 1 < argc) {
            volume = std::stof(argv[++i]);
        } else if (arg.rfind("--volume=", 0) == 0) {
            volume = std::stof(arg.substr(std::string("--volume=").size()));
        }
    }

    auto device = std::make_unique<Device::Device_t>();
    if (volume >= 0.0f) device->setVolume(volume);
    device->run();
    return 0;
}
