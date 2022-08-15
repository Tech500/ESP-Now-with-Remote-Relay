/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-one-to-many-esp32-esp8266/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  Receiver only!!!!!!!!!!!!   Adafruit Feather
*********/

#include <esp_now.h>
#include <WiFi.h>

// ACCESS POINT credentials
const char* ssidAP = "ESP32-Access-Point";
const char* passwordAP = "123456789";

int relay;

//Structure to send data
//Must match the receiver structure
typedef struct struct_message2 {
    int batteryRelay;
} struct_message2;

// Create a struct_message called incomingReadings2 to hold relay status.
struct_message2 remoteRelay;

esp_now_peer_info_t peerInfo;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("\nPacket received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println("");
  Serial.println(macStr);
  memcpy(&remoteRelay, incomingData, sizeof(remoteRelay));
  relay = remoteRelay.batteryRelay;
  Serial.print("\nRelay Status:  ");
  Serial.println(relay);
  
}

void setup() {
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssidAP,passwordAP,5);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {}