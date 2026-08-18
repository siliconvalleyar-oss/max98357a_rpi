#ifndef HMC5883L_HPP
#define HMC5883L_HPP

#include <cstdint>

namespace Device {
namespace Drivers {

class HMC5883L {
public:
    HMC5883L();
    ~HMC5883L();
    bool init();
    void read(int16_t* x, int16_t* y, int16_t* z);
    void close();

private:
    bool initialized;
};

} // namespace Drivers
} // namespace Device

#endif
