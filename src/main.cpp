#include <memory>
#include <iostream>
#include <string>

#include "Device_t.hpp"

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--version") {
            std::cout << "App v" << VERSION << std::endl;
            return 0;
        }
    }

    auto device = std::make_unique<Device::Device_t>();
    device->run();
    return 0;
}
