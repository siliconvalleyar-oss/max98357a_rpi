#ifndef GPIO_HPP
#define GPIO_HPP

#include <cstdint>

namespace Device {
namespace Drivers {

class GPIO {
public:
    static void setup(uint8_t pin, uint8_t mode);
    static void write(uint8_t pin, uint8_t value);
    static uint8_t read(uint8_t pin);
};

} // namespace Drivers
} // namespace Device

#endif
