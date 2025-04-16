#include "myPot.h"

MyPot::MyPot(uint8_t pin)
: _pin(pin)
{}

void MyPot::begin() {
    pinMode(_pin, INPUT); // Configure as ESP32 ADC1 pin
}

PotLevel MyPot::getLevel() {
    int rawVal = analogRead(_pin); // 0~4095 (ESP32 default is 12-bit ADC)

    if (rawVal < 50) {
        Serial.println("PotLevel: LEVEL_STOP");
        return PotLevel::LEVEL_STOP;   // Stop
    } else if (rawVal < 4050) { 
        Serial.println("PotLevel: LEVEL_CLEAN");
        return PotLevel::LEVEL_CLEAN;  // Clean
    } else {
        Serial.println("PotLevel: LEVEL_CLEAN_HARDER");
        return PotLevel::LEVEL_HARDER; // Harder
    }
}
