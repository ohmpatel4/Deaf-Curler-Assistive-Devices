#pragma once
#include <Arduino.h>

class SleepManager {
public:
    SleepManager(unsigned long inactivityMs);
    void resetTimer();
    void update();
private:
    unsigned long _timeout;
    unsigned long _lastActive;
};
