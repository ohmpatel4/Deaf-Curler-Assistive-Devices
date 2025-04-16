#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// =========== Include NeoPixel library ===========
#include <Adafruit_NeoPixel.h>

// =========== NeoPixel Configuration ===========
// 8 LEDs connected to GPIO 32
#define LED_PIN    32
#define MISC_PIN   33        
#define LED_COUNT  8        

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Pairing Configuration
#define MAX_PEER  1
 int peerCount = 0;
 int oldestIndex = 0;
 uint8_t peerList[MAX_PEER][6];
 int pairingMode = 1;



// =========== Group Information ===========

int groups[][2] = {
    {0, 1},  // (1, 2)
    {2, 5},  // (3, 6)
    {3, 4},  // (4, 5)
    {6, 7}   // (7, 8)
};
int groupCount = sizeof(groups) / sizeof(groups[0]);

// =========== Helper Function: Set color for specified group ===========
void setGroupColor(int groupIndex, uint32_t color) {
    for (int i = 0; i < 2; i++) {
        int ledIndex = groups[groupIndex][i];
        strip.setPixelColor(ledIndex, color);
    }
}

// addPeer Helper Function

boolean addPeer(const uint8_t *mac_addr) {

    // If list is full, remove the oldest peer
    if (peerCount >= MAX_PEER) {
      Serial.println("Peer list full. Removing the oldest peer.");
      if (esp_now_del_peer(peerList[oldestIndex]) == ESP_OK) {
        Serial.println("Removed oldest peer");
      }else{
        Serial.println("Error removing old peer.");
        peerCount--;
        return false;
      }
      
      peerCount--;  // Reduce count after removal
    }
    
    
  
    // Now add the new peer to ESP-NOW
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac_addr, 6);
    peerInfo.channel = 1;    
    peerInfo.encrypt = false; 
    
    esp_err_t status = esp_now_add_peer(&peerInfo);
    if (status == ESP_OK) {
      Serial.println("Added new peer to ESP-NOW list.");
      memcpy(&peerList[oldestIndex], mac_addr, 6);
      peerCount++;
      oldestIndex = (oldestIndex + 1)%MAX_PEER;
      return true;
      
    }else if (status == ESP_ERR_ESPNOW_EXIST) {
      Serial.println("Peer already exists in the list.");
      return false;
    } 
    else {
      Serial.printf("Error adding peer: %d\n", status);
      return false;
    }
  }
  

// =========== Enum: Commands received by this receiver ===========
enum RxCommand {
    CMD_STOP = 0,
    CMD_LEFT_LOW,
    CMD_LEFT_HIGH,
    CMD_RIGHT_LOW,
    CMD_RIGHT_HIGH,
    CMD_LEFT_LOW_BOTH,
    CMD_LEFT_HIGH_BOTH,
    CMD_RIGHT_LOW_BOTH,
    CMD_RIGHT_HIGH_BOTH,
    CMD_CLEAN_LOW,
    CMD_CLEAN_LOW_BOTH,
    CMD_CLEAN_HIGH,
    CMD_CLEAN_HIGH_BOTH,
    CMD_LED_OFF,
    CMD_PAIRING,
};

// =========== Structure of received data packet ===========
typedef struct {
    uint8_t cmd;
    char codeWord[10];
} RxPacket;

// =========== ESP-NOW Receive Callback ===========
void onDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    Serial.printf("Received packet from: %02X:%02X:%02X:%02X:%02X:%02X\n",mac_addr[0], mac_addr[1], mac_addr[2],mac_addr[3], mac_addr[4], mac_addr[5]);
    // If received data is smaller than expected packet size, ignore it
    if (len < sizeof(RxPacket)) return;

    // Copy received data into RxPacket structure
    RxPacket rx;
    memcpy(&rx, incomingData, sizeof(rx));

    if (strcmp(rx.codeWord, "Shhhh") == 0) {
        Serial.println("Code word matched.");
    } else {
        Serial.println("Code word did not match. Ignoring packet.");
        return;
    }
    // Extract the command
    uint8_t c = rx.cmd;
    Serial.printf("[RX] Received cmd=%d\n", c);

    // Clear strip to ensure a clean state before drawing
    strip.clear();
    if(c < CMD_PAIRING && peerCount == 0){ //ignore non-pairing Command without any peers 
        return;
    }
    if(c < CMD_PAIRING && peerCount > 0){ //normal command and we have peers
        uint8_t tempc = c;
        c = 15;
        for (int i = 0; i < peerCount; i++) { //check if command is from a peer, otherwise, c = 15 and ignored.
            if (memcmp(peerList[i], mac_addr, 6) == 0) {
              c = tempc;
              i = peerCount;
            }
        }
    }
    // Process the command and update LEDs accordingly
    switch (c) {
        case 0:
            for (int i = 0; i < LED_COUNT; i++) {
                strip.setPixelColor(i, strip.Color(255, 0, 0)); // Red: R=255, G=0, B=0
            }
            break;

        case 1:
            // CMD_LEFT_LOW: group 0 = blue, group 2 = green (left light sweep)
            setGroupColor(0, strip.Color(0, 0, 255)); // Blue
            setGroupColor(2, strip.Color(0, 255, 0));   
            break;

        case 2:
            // CMD_LEFT_HIGH: group 0 = blue, group 2 = red (left heavy sweep)
            setGroupColor(0, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(255, 0, 0));    
            break;    

        case 3:
            // CMD_RIGHT_LOW: group 3 = blue, group 2 = green (right light sweep)
            setGroupColor(3, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(0, 255, 0));   
            break;

        case 4:
            // CMD_RIGHT_HIGH: group 3 = blue, group 2 = red (right heavy sweep)
            setGroupColor(3, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(255, 0, 0));    
            break;

        case 5:
            // CMD_LEFT_LOW_BOTH: group 0 = blue, group 2 = green (left light sweep)
            // group 1 = purple (Both Sweepers)
            setGroupColor(0, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(0, 255, 0));
            setGroupColor(1, strip.Color(128, 0, 128));     
            break;

        case 6:
            // CMD_LEFT_HIGH_BOTH: group 0 = blue, group 2 = red (left heavy sweep)
            // group 1 = purple (Both Sweepers)
            setGroupColor(0, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(255, 0, 0));
            setGroupColor(1, strip.Color(128, 0, 128));      
            break;    

        case 7:
            // CMD_RIGHT_LOW_BOTH: group 3 = blue, group 2 = green (right light sweep)
            // group 1 = purple (Both Sweepers)
            setGroupColor(3, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(0, 255, 0));
            setGroupColor(1, strip.Color(128, 0, 128));     
            break;

        case 8:
            // CMD_RIGHT_HIGH_BOTH: group 3 = blue, group 2 = red (right heavy sweep)
            // group 1 = purple (Both Sweepers)
            setGroupColor(3, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(255, 0, 0));
            setGroupColor(1, strip.Color(128, 0, 128));   
            break;

        case 9:
            // CMD_CLEAN_LOW: group 2 = green (clean only)
            setGroupColor(2, strip.Color(0, 255, 0));     
            break;

        case 10:
            // CMD_CLEAN_LOW_BOTH: group 2 = green (clean only)
            // group 1 = purple (Both Sweepers)
            setGroupColor(2, strip.Color(0, 255, 0));     
            setGroupColor(1, strip.Color(128, 0, 128));   
            break;

        case 11:
            // CMD_CLEAN_HIGH: group 2 = red (clean only)
            // group 1 = purple (Both Sweepers)
            setGroupColor(2, strip.Color(255, 0, 0));      
            break; 
            
        case 12:
            // CMD_CLEAN_HIGH_BOTH: group 2 = red (clean only)
            // group 1 = purple (Both Sweepers)
            setGroupColor(2, strip.Color(255, 0, 0));      
            setGroupColor(1, strip.Color(128, 0, 128));   
        break;   

        case 13:
            //LEDS OFF: turn off all LEDs
            for (int i = 0; i < LED_COUNT; i++) {
                strip.setPixelColor(i, strip.Color(0, 0, 0)); 
            }    
            break;

        case CMD_PAIRING: {
            //pairing command
            for (int i = 0; i < peerCount; i++) {
                if (memcmp(peerList[i], mac_addr, 6) == 0) {
                    RxPacket packet;
                    packet.cmd = static_cast<uint8_t>(0);
                    strcpy(packet.codeWord, "Shhhh");
                    esp_err_t result = esp_now_send(mac_addr, (uint8_t *)&packet, sizeof(packet));
                  Serial.println("Peer already in list. Ignoring packet.");
                  return; // Exit the function if the peer is already added
                }
            }
            bool status = addPeer(mac_addr);
            if(status){
                RxPacket packet;
                packet.cmd = static_cast<uint8_t>(0);
                strcpy(packet.codeWord, "Shhhh");
                esp_err_t result = esp_now_send(mac_addr, (uint8_t *)&packet, sizeof(packet));
                if (result == ESP_OK) {
                    Serial.println("Pairing response sent successfully");
                } else {
                    Serial.printf("Error sending pairing response: %d\n", result);
                }
            }
            break;
        }
        default:
            // Undefined command: do nothing
            break;
    }

    // After setting the colors, refresh the strip
    strip.show();
}


void setup() {
    Serial.begin(115200);

    // Initialize NeoPixel
    strip.begin();
    strip.show(); // Clear all LEDs
    Serial.println("[RX] NeoPixel init OK");
    Serial.print("Device MAC Address: ");
    Serial.println(WiFi.macAddress());
    // Configure WiFi
    WiFi.mode(WIFI_MODE_STA);

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("[RX] ESPNOW init failed, restarting...");
        ESP.restart();
    }

    // Register receive callback
    esp_now_register_recv_cb(onDataRecv);

    Serial.println("[RX] Setup done.");
}

void loop() {
    // Light sleep can be added here
    //delay(1000);
}
