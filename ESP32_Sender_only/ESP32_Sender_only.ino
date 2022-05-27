/*****************************************************************

   Board 1 Sender only:  Node32s

   ESP32_SenderCombo_WIP.ino  05/19/2022 @ 08:11 (Work in progress)
   C:\Users\1234\Desktop\â– â–    wip Work in Progress\ESP32_Sender_Combo_wip\ESP32_Sender_Combo_wip.ino

   William Lucid, Tech500

   Board:  HiLetGo ESP32 (Node32s board type)
   Arduino IDE Core 1.04
   Awake time 3.8 Secxonds 
   Sleep time 15 Minutes
  
*/////////////////////////////////////////////////////////////////

#include <esp_now.h>
#include <WiFi.h>
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>
#include <BME280I2C.h>                //Use the Arduino Library Manager, get BME280 by Tyler Glenn
#include <EnvironmentCalculations.h>  //Use the Arduino Library Manager, get BME280 by Tyler Glenn
#include <Wire.h>    //Part of version 1.0.4 ESP32 Board Manager install  -----> Used for I2C protocol


//The gateway access point credentials
const char* APssid = "ESP32-Access-Point";
const char* APpassword = "123456789";

// Replace with your network details
const char* ssid = "#######";
const char* password = "########";

//setting the addresses
IPAddress staticIP(10, 0, 0, 202);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(10,0,0,1);
IPAddress secondaryDNS(10,0,0,1);

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

// this is the MAC Address of the remote ESP server which receives these sensor readings
uint8_t Broadcast[] = {0xB8, 0xF0, 0x09, 0x89, 0x2C, 0x68};   //ThingPulse

//Wi-Fi channel (must match the gateway wi-fi channel as an access point)
#define CHAN_AP 5

// How many minutes the ESP should sleep
#define DEEP_SLEEP_TIME 15 //Minutes

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message1 {
  int id;
  float temp;
  float heat;
  float hum;
  float dew;
  float press;     
  int readingId;
} struct_message1;

//Create a struct_message called myData
struct_message1 myData;

//MAC Address of the receiver
uint8_t broadcastAddress[] = {0xB8, 0xF0, 0x09, 0x89, 0x2C, 0x68};   //ThingPulse

//MAC:  30:AE:A4:DF:B3:6C   Adafruit Dev Broad

//MAC:  B8:F0:09:89:2C:68   ThingPulse

//MAC:  40:91:51:9A:1D:4C   Node32s #1


//BME280

// Assumed environmental values:
float referencePressure = 1021.1;   // hPa local QFF (official meteor-station reading) ->  KEYE, Indianapolis, IND
float outdoorTemp = 35.6;           // Â°F  measured local outdoor temp.
float barometerAltitude = 250.698;  // meters ... map readings + barometer position  -> 824 Feet  Garmin, GPS measured Altitude.

BME280I2C::Settings settings(
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::Mode_Forced,
  BME280::StandbyTime_1000ms,
  BME280::Filter_16,
  BME280::SpiEnable_False,
  BME280I2C::I2CAddr_0x76);

BME280I2C bme(settings);

float temp(NAN), temperature, hum(NAN), pres(NAN), heatIndex, dewPoint, absHum, altitude, seaLevel;

float currentPressure;
float pastPressure;
float difference;  //change in barometric pressure drop; greater than .020 inches of mercury.
float heatId;      //Conversion of heatIndex to Farenheit
float dewPt;       //Conversion of dewPoint to Farenheit
float altFeet;     //Conversion of altitude to Feet

unsigned long previousMillis = 0;   // Stores last time temperature was published

unsigned int readingId = 0;

unsigned long delayTime;

RTC_DATA_ATTR int bootCount = 0;

RTC_DATA_ATTR float elapsedMicros = 0;

RTC_DATA_ATTR float seconds = 0;

float startMicros;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n\n", wakeup_reason); break;
  }

}

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t\n");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println("");
}//volatile boolean callbackCalled;

WiFiClient client;

#define yellow_LED  26

char strftime_buf[64];

String dtStamp(strftime_buf);

void setup() {

  startMicros = micros();

  Serial.begin(9600);

  while (!Serial);

  Serial.println();

  pinMode(yellow_LED,OUTPUT);
  
  digitalWrite(yellow_LED,HIGH);

  WiFi.persistent( false ); // for time saving

  // Connecting to local WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS);
  WiFi.begin(ssid, password);
  delay(10);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Server IP:  ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  WiFi.disconnect(true);

  Serial.print("WiFi SSID Disconnected:  ");
  Serial.println(ssid);
  Serial.println("");

  WiFi.mode(WIFI_OFF);

  // read sensor first before awake generates heat
  getWeatherData();

  //Set device as a Wi-Fi Station
  WiFi.persistent( false ); // for time saving ---setup Access Point
  WiFi.mode(WIFI_STA);
  WiFi.begin(APssid, APpassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Access Point...");
  }

  Wire.begin(21, 22);

  while (!bme.begin()) {
    Serial.println("");
    Serial.println("Could not find BME280 sensor!");
    delayTime = 1000;
  }

  // bme.chipID(); // Deprecated. See chipModel().
  switch (bme.chipModel())  {
    case BME280::ChipModel_BME280:
      Serial.println("");
      Serial.println("Found BME280 sensor! Success.");
      break;
    case BME280::ChipModel_BMP280:
      Serial.println("");
      Serial.println("Found BMP280 sensor! No Humidity available.");
      break;
    default:
      Serial.println("");
      Serial.println("Found UNKNOWN sensor! Error!");
  }

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  //Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = CHAN_AP;
  peerInfo.encrypt = false;

  //Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Register for a callback function that will be called when data is received
  //esp_now_register_recv_cb(OnDataRecv);

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  float seconds = elapsedMicros / 1000000;  //Convert to seconds
  Serial.print(" Seconds:  ");
  Serial.print(seconds, 1);
  Serial.println("  Awake time");
  Serial.println("");  

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

    /*
    First we configure the wake up source
    We set our ESP32 to wake up every 5 seconds
  */

  // Configure the timer to wake us up!
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 60L * 1000000L);

  Serial.println("");
  Serial.println("ESP32_Sender going into Deep Sleep for:  " + String(DEEP_SLEEP_TIME) + " Minutes");
  Serial.println("");
  
  /*
    Next we decide what all peripherals to shut down/keep on
    By default, ESP32 will automatically power down the peripherals
    not needed by the wakeup source, but if you want to be a poweruser
    this is for you. Read in detail at the API docs
    http://esp-idf.readthedocs.io/en/latest/api-reference/system/deep_sleep.html
    Left the line commented as an example of how to configure peripherals.
    The line below turns off all RTC peripherals in deep sleep.
  */
  //esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  //Serial.println("Configured all RTC Peripherals to be powered down in sleep");

  /*
    Now that we have setup a wake cause and if needed setup the
    peripherals state in deep sleep, we can now start going to
    deep sleep.
    In the case that no wake up sources were provided but deep
    sleep was started, it will sleep forever unless hardware
    reset occurs.
  */

   goToDeepSleep();

   Serial.println("This will never be printed");

}

void loop() {}

void getWeatherData() 
{
	
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);

  bme.read(pres, temp, hum, tempUnit, presUnit);

  EnvironmentCalculations::AltitudeUnit envAltUnit = EnvironmentCalculations::AltitudeUnit_Meters;
  EnvironmentCalculations::TempUnit envTempUnit = EnvironmentCalculations::TempUnit_Celsius;

  delay(300);

  /// To get correct local altitude/height (QNE) the reference Pressure
  ///    should be taken from meteorologic messages (QNH or QFF)
  /// To get correct seaLevel pressure (QNH, QFF)
  ///    the altitude value should be independent on measured pressure.
  /// It is necessary to use fixed altitude point e.g. the altitude of barometer read in a map
  absHum = EnvironmentCalculations::AbsoluteHumidity(temp, hum, envTempUnit);
  altitude = EnvironmentCalculations::Altitude(pres, envAltUnit, referencePressure, outdoorTemp, envTempUnit);
  dewPoint = EnvironmentCalculations::DewPoint(temp, hum, envTempUnit);
  heatIndex = EnvironmentCalculations::HeatIndex(temp, hum, envTempUnit);
  seaLevel = EnvironmentCalculations::EquivalentSeaLevelPressure(barometerAltitude, temp, pres, envAltUnit, envTempUnit);
  heatId = (heatIndex * 1.8) + 32;
  dewPt = (dewPoint * 1.8) + 32;
  altFeet = 843;

  temperature = (temp * 1.8) + 32;  //Convert to Fahrenheit

  currentPressure = seaLevel * 0.02953;  //Convert from hPa to in Hg.

  //Set values to send
  myData.id = BOARD_ID;
  myData.temp = temperature;
  myData.heat = heatId;     
  myData.hum = hum;
  myData.dew = dewPt;
  myData.press = currentPressure;
  myData.readingId = readingId++;

}

void goToDeepSleep()
{

  getWeatherData();

  //Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) 
  {

    Serial.println("Sent with success");

  }
  else 
  {

    Serial.println("Error sending the data");
  
  }

  delayTime = 2000;

  if (result == ESP_OK) 
  {

    Serial.println("Going to sleep now");
    
    elapsedMicros = micros() - startMicros;

    digitalWrite(yellow_LED,LOW);

    Serial.flush();

  }
  
  //Serial.println("Going to sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  adc_power_off();
  esp_wifi_stop();
  esp_bt_controller_disable();

  // Go to sleep! Zzzz
  esp_deep_sleep_start();

}
