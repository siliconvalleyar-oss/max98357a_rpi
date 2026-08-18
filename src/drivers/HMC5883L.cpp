#include "HMC5883L.hpp"

namespace Device {
namespace Drivers {

HMC5883L::HMC5883L() : initialized(false) {}
HMC5883L::~HMC5883L() {}

bool HMC5883L::init() { initialized = true; return true; }
void HMC5883L::read(int16_t* x, int16_t* y, int16_t* z) {}
void HMC5883L::close() { initialized = false; }

} // namespace Drivers
} // namespace Device
