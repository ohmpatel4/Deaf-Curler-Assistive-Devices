#pragma once
#include <Arduino.h>

// Define three levels for potentiometer input
enum class PotLevel : uint8_t {
    LEVEL_STOP = 0,
    LEVEL_CLEAN,
    LEVEL_HARDER
};

// Potentiometer class: divides the analog value into three ranges
class MyPot {
public:
    MyPot(uint8_t pin);
    void begin();
    PotLevel getLevel();

private:
    uint8_t _pin;
};
