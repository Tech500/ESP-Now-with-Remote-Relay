/*

Board 3 Receiver Only

  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <esp_now.h>
#include <WiFi.h>

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Combo #2 = BOARD_ID 2, ESP32 Receiver #3 = BOARD_ID 3etc)
#define BOARD_ID 3

//Store incoming relay status
int relay;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message2 {
  int batteryRelay;
} struct_message2;

// Create a struct_message2 called incomingReadings2
struct_message2 incomingReadings2;

esp_now_peer_info_t peerInfo;

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings2, incomingData, sizeof(incomingReadings2));
  relay = incomingReadings2.batteryRelay;
  
}

void setup() {
  // Init Serial Monitor
  Serial.begin(9600);
 
  while(! Serial){};
  
    // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_STA);
    
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);


}
 
void loop() 
{

  struct_message2 incomingReadings2;
  incomingReadings2.batteryRelay = relay;

  if(relay == 0)
  {

    Serial.println("Relay is OFF\n");

    relay = 3;

  }

  if(relay == 1)
  {

    Serial.println("Relay is ON\n");

    relay = 3;

  }

}
 
