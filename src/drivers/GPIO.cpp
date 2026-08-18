#include "GPIO.hpp"
#include <bcm2835.h>

namespace Device {
namespace Drivers {

void GPIO::setup(uint8_t pin, uint8_t mode) {
    if (mode == 0) bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
    else bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
}

void GPIO::write(uint8_t pin, uint8_t value) {
    bcm2835_gpio_write(pin, value);
}

uint8_t GPIO::read(uint8_t pin) {
    return bcm2835_gpio_lev(pin);
}

} // namespace Drivers
} // namespace Device
