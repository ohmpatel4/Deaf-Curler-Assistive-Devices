#pragma once
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define MAX_PEERS 3
#define LIGHTSLEEP_TIME 150000

// Define command types - a combination of button and potentiometer input
enum class TxCommand : uint8_t {
    STOP            = 0,
    LEFT_LOW        = 1,
    LEFT_HIGH       = 2,
    RIGHT_LOW       = 3,
    RIGHT_HIGH      = 4,
    LEFT_LOW_BOTH   = 5,
    LEFT_HIGH_BOTH  = 6,
    RIGHT_LOW_BOTH  = 7,
    RIGHT_HIGH_BOTH = 8,
    CLEAN_LOW       = 9,
    CLEAN_LOW_BOTH  = 10,
    CLEAN_HARD      = 11,
    CLEAN_HARD_BOTH = 12,
    LED_OFF         = 13,
    PAIRING         = 14,
};
//global variables
extern bool transmitting;

extern int peerCount;
extern int oldestIndex;
extern int newIndex;
extern uint8_t peerList[MAX_PEERS][6];
extern volatile bool transmitterState; 
extern bool sleeper;


//Broadcast mode Address
extern uint8_t broadcastAddress[6]; 
// Structure of the data packet to be sent
struct TxPacket {
    uint8_t cmd;
    char codeWord[10];
};

class EspNowSender {
public:
    EspNowSender();
    void begin();
    void sendCommand(TxCommand cmd);

private:
    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void onDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len);
    uint8_t _peerMac[6];
    static boolean addPeer(const uint8_t *mac_addr);
};
