#ifndef DEVICE_T_HPP
#define DEVICE_T_HPP

#include <string>

namespace Device {

class Device_t {
public:
    Device_t();
    ~Device_t();
    void run();

private:
    void showVersion() const;
    void initBcm2835();
    void initOLED();
    void initAudio();
    void playTestTone();
    void playMp3Folder();
    void cleanup();
    void displayInfo();

    bool oled_initialized;
    std::string mp3_dir;
};

} // namespace Device

#endif
