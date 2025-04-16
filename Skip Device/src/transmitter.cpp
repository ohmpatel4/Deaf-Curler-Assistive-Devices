#include <Arduino.h>
#include "myButton.h"
#include "myPot.h"
#include "myEspNowSender.h"
#include <WiFi.h>

// ========== Define Pins ==========
static const uint8_t PIN_POT = 36; // Potentiometer pin (ESP32 ADC1)
static const uint8_t PIN_BTN_LEFT  = 14;
static const uint8_t PIN_BTN_RIGHT = 17;
static const uint8_t PIN_BTN_BOTH  = 16;
static const uint8_t PIN_BTN_PAIRING = 18;

// ========== Send interval (milliseconds) ==========
static const unsigned long SEND_INTERVAL = 150;

// ========== Enter sleep if no input after this timeout ==========
static const unsigned long INACTIVITY_TIMEOUT = 30000; // 30 seconds

// P2P MAC address
uint8_t peerMac[6] = {0xEC, 0x64, 0xC9, 0x5D, 0xD8, 0xF8};


// Real-time button states
static bool leftDown  = false;
static bool rightDown = false;
static bool bothDown  = false;  // "both" button


// Global objects
MyButton btnLeft(PIN_BTN_LEFT);
MyButton btnRight(PIN_BTN_RIGHT);
MyButton btnBoth(PIN_BTN_BOTH);
MyButton btnPair(PIN_BTN_PAIRING);
MyPot       pot(PIN_POT);
EspNowSender espnow;


unsigned long lastSendTime = 0;
bool leftPressedPrev  = false;
bool rightPressedPrev = false;
volatile bool transmitterState = false;
bool transmitting = false;
bool sleeper = false;
uint8_t peerList[MAX_PEERS][6] = {0};
int peerCount = 0;
int oldestIndex = 0;
int newIndex = 0;
uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


// Helper function: generate the command to send based on potentiometer level and button state
TxCommand makeCommand(PotLevel level, bool leftDown, bool rightDown, bool bothDown) {
    // LeftDown + Low => LEFT_LOW,  LeftDown + High => LEFT_HIGH
  //  Serial.printf("Level:%.2f LeftDown:%d  rightDown:%d, Both:%d\n", (double)level, leftDown, rightDown, bothDown);
    if (level == PotLevel::LEVEL_STOP) {
        return TxCommand::STOP;
    }
    switch (level) {
    case PotLevel::LEVEL_CLEAN:
        if (!bothDown) {
            // If left button is pressed
            if (leftDown && !rightDown) {
                return TxCommand::LEFT_LOW;
            }
            // If right button is pressed
            if (rightDown && !leftDown) {
                return TxCommand::RIGHT_LOW;
            }
            if (!rightDown && !leftDown) {
                return TxCommand::CLEAN_LOW;
            }
        } else {
            if (leftDown && !rightDown) {
                return TxCommand::LEFT_LOW_BOTH;
            }
            if (rightDown && !leftDown) {
                return TxCommand::RIGHT_LOW_BOTH;
            }
            if (!rightDown && !leftDown) {
                return TxCommand::CLEAN_LOW_BOTH;
            }
        }
        
        break;

    case PotLevel::LEVEL_HARDER:
        // Mid level => left => LEFT_HIGH, right => RIGHT_HIGH, none => CLEAN_HARD
        if (!bothDown) {
            if (leftDown && !rightDown) {
                return TxCommand::LEFT_HIGH;
            }
            if (rightDown && !leftDown) {
                return TxCommand::RIGHT_HIGH;
            }
            if (!rightDown && !leftDown) {
                return TxCommand::CLEAN_HARD;
            }
        } else {
            if (leftDown && !rightDown) {
                return TxCommand::LEFT_HIGH_BOTH;
            }
            if (rightDown && !leftDown) {
                return TxCommand::RIGHT_HIGH_BOTH;
            }
            if (!rightDown && !leftDown) {
                return TxCommand::CLEAN_HARD_BOTH;
                //hello dill pickle
            }
        }
        
        break;
    }
    return TxCommand::LED_OFF;
}

void setup() {
    Serial.begin(115200);

    // Initialize all modules
    btnLeft.begin();
    btnRight.begin();
    btnBoth.begin();
    btnPair.begin();
    pot.begin();
    espnow.begin();
    Serial.print("Device MAC Address: ");
    Serial.println(WiFi.macAddress());

    Serial.println("[MAIN] Setup done.");
}

void lightSleep(){
    Serial.printf("Going to sleep for %dms\n", LIGHTSLEEP_TIME/1000);
    esp_wifi_stop();
    esp_sleep_enable_timer_wakeup(LIGHTSLEEP_TIME); 
    esp_light_sleep_start(); 
    esp_wifi_start();  
    sleeper = false;
}

void loop() {
    
    if(transmitting){
       // Serial.println("Currently Transmitting, exiting loop");
        return;
    }

    if(sleeper && !transmitting){
        lightSleep();
    }
    bool pairDebouncing = btnPair.update();
    if(pairDebouncing){
        return;
    }
    if (btnPair.isPressedEvent()) {
        transmitterState = !transmitterState;
        Serial.printf("Pairing mode: %s\n", transmitterState ? "ON" : "OFF");
    }

    if(!transmitterState){ //Normal Mode
        if(peerCount == 0){ //no peers, don't send anything, go to sleep.
            Serial.println("Normal Mode: No peers in list, going to sleep...");
            sleeper = true;
            return;
        }
        unsigned long now = millis();
        // 1. Update button states (with MyButton class debounce and event detection)
        bool leftDebouncing  = btnLeft.update();
        bool rightDebouncing = btnRight.update();
        bool bothDebouncing  = btnBoth.update();
        if(leftDebouncing || rightDebouncing || bothDebouncing){
            return;
        }
        // 2. If any button event (press or release), reset inactivity timer
        bool leftEvent  = btnLeft.isPressedEvent()  || btnLeft.isReleasedEvent();
        bool rightEvent = btnRight.isPressedEvent() || btnRight.isReleasedEvent();
        bool bothEvent  = btnBoth.isPressedEvent()  || btnBoth.isReleasedEvent();
        if (leftEvent || rightEvent || bothEvent) {

        }

        // 3. Check potentiometer level change; if changed, reset timer
        static PotLevel prevLevel = PotLevel::LEVEL_STOP;
        PotLevel currentLevel = pot.getLevel();
        if (currentLevel != prevLevel) {
            prevLevel = currentLevel;

        }

        // 4. Read current button states
        // Use digitalRead() assuming stable values after debouncing
        // (could add a method in MyButton to get stable state)
        bothDown  = (digitalRead(PIN_BTN_BOTH)  == LOW);
        leftDown  = (digitalRead(PIN_BTN_LEFT)  == LOW);
        rightDown = (digitalRead(PIN_BTN_RIGHT) == LOW);
        
        // 5. Generate command based on pot and button state
        TxCommand cmd = makeCommand(currentLevel, leftDown, rightDown, bothDown);

        // 6. Send command at set interval
        if (now - lastSendTime >= SEND_INTERVAL) {
            lastSendTime = now;
            espnow.sendCommand(cmd);
        }

        // 7. Check for sleep timeout

    } else{
        unsigned long now = millis();
        TxCommand cmd = TxCommand::PAIRING;
        // 6. Send command at set interval
        if (now - lastSendTime >= SEND_INTERVAL) {
            lastSendTime = now;
            espnow.sendCommand(cmd);
        }
    }
    
}

