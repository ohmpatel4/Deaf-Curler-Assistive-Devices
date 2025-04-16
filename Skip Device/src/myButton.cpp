#include "myButton.h"

MyButton::MyButton(uint8_t pin)
: _pin(pin)
, _lastSteadyState(HIGH)      // Assume initially not pressed (for INPUT_PULLUP, default is HIGH)
, _lastFlickerableState(HIGH)
, _lastDebounceTime(0)
, _pressedEventFlag(false)
, _releasedEventFlag(false)
, _pressedTime(0)
, _releasedTime(0)
{
}

void MyButton::begin() {
    // Use internal pull-up resistor. If using external pull-up/pull-down, change to INPUT or other as needed
    pinMode(_pin, INPUT_PULLUP);

    // Read initial state to avoid false trigger from noise on startup
    int reading = digitalRead(_pin);
    _lastSteadyState = reading;
    _lastFlickerableState = reading;
    _lastDebounceTime = millis();
}

bool MyButton::update() {
    unsigned long currentTime = millis();
    int reading = digitalRead(_pin);

    // Clear press/release event flags for this cycle
    _pressedEventFlag = false;
    _releasedEventFlag = false;

    // If current reading differs from last flickerable state, debounce starts
    if (reading != _lastFlickerableState) {
        _lastFlickerableState = reading;
        _lastDebounceTime = currentTime;
    }

    // Check if still within debounce period
    bool isDebouncing = (currentTime - _lastDebounceTime < DEBOUNCE_MS);

    // If debounce period passed, treat reading as stable
    if (!isDebouncing) {
        // If the stable reading has changed, a press or release occurred
        if (_lastSteadyState != reading) {
            // Assume: pressed = LOW, released = HIGH
            if (_lastSteadyState == HIGH && reading == LOW) {
                // Transition from released to pressed
                _pressedTime = currentTime;
                _pressedEventFlag = true;  
            }
            else if (_lastSteadyState == LOW && reading == HIGH) {
                // Transition from pressed to released
                _releasedTime = currentTime;
                _releasedEventFlag = true;
            }
            // Update last stable reading
            _lastSteadyState = reading;
        }
    }

    return isDebouncing;
}

bool MyButton::isPressedEvent() {
    // Return true if update() detected a press event (one-time flag)
    return _pressedEventFlag;
}

bool MyButton::isReleasedEvent() {
    // Return true if update() detected a release event (one-time flag)
    return _releasedEventFlag;
}

unsigned long MyButton::getPressDuration() {
    // Duration between last press and release
    // If button hasn't been released yet, return previous release duration
    return (_releasedTime - _pressedTime);
}
