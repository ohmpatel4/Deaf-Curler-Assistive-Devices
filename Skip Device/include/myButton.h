#pragma once
#include <Arduino.h>

// Simple button class: includes debounce handling, and records press/release timestamps
class MyButton {
public:
    MyButton(uint8_t pin);

    void begin();
    bool update();            // Call in each loop iteration to handle debouncing; returns whether it is currently in the debounce period
    bool isPressedEvent();    // Returns true if a press event (LOW to HIGH or configurable) just occurred
    bool isReleasedEvent();   // Returns true if a release event (HIGH to LOW) just occurred
    unsigned long getPressDuration();

private:
    uint8_t  _pin;
    int      _lastSteadyState;
    int      _lastFlickerableState;
    bool     _pressedEventFlag;
    bool     _releasedEventFlag;
    unsigned long _lastDebounceTime;
    unsigned long _pressedTime;
    unsigned long _releasedTime;

    static const unsigned long DEBOUNCE_MS = 50;      // Debounce duration
};
