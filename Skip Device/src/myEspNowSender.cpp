#include "myEspNowSender.h"



EspNowSender::EspNowSender() {
    memset(_peerMac, 0, 6);
}

void EspNowSender::begin() {


    // Set WiFi mode
    WiFi.mode(WIFI_MODE_STA);
    // WiFi.disconnect(true);

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("[TX] ESP-NOW init failed, reboot...");
        ESP.restart();
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 1;
    peerInfo.encrypt = false;
    
    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    // Register the send callback
    esp_now_register_send_cb(EspNowSender::onDataSent);

    //Register the Receive callback
    esp_now_register_recv_cb(EspNowSender::onDataRecv);

}

void EspNowSender::sendCommand(TxCommand cmd) {
    TxPacket packet;
    packet.cmd = static_cast<uint8_t>(cmd);
    strcpy(packet.codeWord, "Shhhh");
    if(!transmitterState){ //Normal Mode
        for(int i = 0; i < peerCount; i++){
            transmitting = true;
            esp_err_t result = esp_now_send(peerList[i], (uint8_t*)&packet, sizeof(packet));
            if (result == ESP_OK) {
                Serial.printf("[TX] send cmd=%d OK\n", packet.cmd);
            } else {
                Serial.printf("[TX] send cmd=%d FAIL, err=%d\n", packet.cmd, result);
                transmitting = false;
            }
        }
        sleeper = true;
    } else{ //pairing mode, send to broadcast address
        transmitting = true;
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&packet, sizeof(packet));
            if (result == ESP_OK) {
                Serial.printf("[TX] send cmd=%d OK\n", packet.cmd);
            } else {
                Serial.printf("[TX] send cmd=%d FAIL, err=%d\n", packet.cmd, result);
                transmitting = false;
            }
    }
    
}

void EspNowSender::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("[transmitter] send result: ");
    if (status == ESP_NOW_SEND_SUCCESS) {
        Serial.println("Success");
  } else {
    Serial.println("Fail");
  }
  transmitting = false; 
}



boolean EspNowSender::addPeer(const uint8_t *mac_addr) {
    // If list is full, remove the oldest peer
    if (peerCount >= MAX_PEERS) {
      Serial.println("Peer list full. Removing the oldest peer.");
      esp_err_t result = esp_now_del_peer(peerList[oldestIndex]); 
      Serial.println(result); 
      if(result == ESP_OK){
        Serial.println("Removed oldest peer");
        oldestIndex = (oldestIndex + 1)%MAX_PEERS;
      }else{
        Serial.println("Error removing old peer.");
        return false;
      }
      peerCount--;  // Reduce count after removal
    }
    
    
  
    // Now add the new peer to ESP-NOW
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac_addr, 6);
    peerInfo.channel = 0;    
    peerInfo.encrypt = false; 
    
    esp_err_t status = esp_now_add_peer(&peerInfo);
    if (status == ESP_OK) {
      Serial.println("Added new peer to ESP-NOW list.");
      memcpy(&peerList[newIndex], mac_addr, 6);
      peerCount++;
      newIndex = (newIndex +1)%MAX_PEERS;
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

  void EspNowSender::onDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
    // Print the sender's MAC address
    Serial.printf("Received packet from: %02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0], mac_addr[1], mac_addr[2],mac_addr[3], mac_addr[4], mac_addr[5]);
    if (len < sizeof(TxPacket)) return;

    // Copy received data into RxPacket structure
    TxPacket rx;
    memcpy(&rx, incomingData, sizeof(rx));

    if (strcmp(rx.codeWord, "Shhhh") == 0) {
        Serial.println("Code word matched.");
        boolean status = addPeer(mac_addr);
    } else {
        Serial.println("Code word did not match. Ignoring packet.");
        return;
    }
  }