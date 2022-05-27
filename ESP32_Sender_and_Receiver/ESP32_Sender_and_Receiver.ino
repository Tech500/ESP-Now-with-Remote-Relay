///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                       Board  2 Receiver and Sender: ThingPulse
//
//                       
//                       Version 1.0 "ESP32_Receiver.ino"  5/26/2022 @ 011:31 EDT Developed by William Lucid, tech500
//                      
//                       Developing AsyncWebServer 11/07/2019; modifying with fileRead and not found function.  Adding wifi log of reconnects.
//
//                       Portion of NTP time code was developed from code provided by schufti  --of ESP8266Community
//
//                       listFiles and readFile functions by martinayotte of ESP8266 Community Forum.  readFile function modified by RichardS of ESP8266 Community Forum for ESP32.
//
//                       Time keeping functions uses NTP Time.
//
//                       GPS and rain gauge code developed by Muhammad Haroon.  Thank you Muhammad.
//
//                       Previous projects:  https://github.com/tech500 
//
//                       Project is Open-Source, requires one BME280 breakout board, a NEO m8n GPS Module, and a "HiLetgo ESP-WROOM-32 ESP32 ESP-32S Development Board"
//
//                       http://weather-3.ddns.net/Weather  Project web page  --Servered from ESP32.
//
//                       https://observedweather.000webhostapp.com/index.php  --Servered from "Free" Domain Hosting service.
//
//                       Note:  Uses esp32 core by ESP32 Community version 1.0.4 from "Arduino IDE, Board Manager"   Arduino IDE; use Board:  "Node32s" for the "HiLetGo" ESP32 Board.
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////


// ********************************************************************************
// ********************************************************************************
//
//   See library downloads for each library license.
//
// ********************************************************************************
// ********************************************************************************


#include <esp_now.h>
#include "EEPROM.h"  //Part of version 1.0.4 ESP32 Board Manager install
#include <WiFi.h>   //Part of version 1.0.4 ESP32 Board Manager install
#include <HTTPClient.h>  //Part of version 1.0.4 ESP32 Board Manager install
#include <AsyncTCP.h>  //https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h>  //https://github.com/me-no-dev/ESPAsyncWebServer
#include <Arduino_JSON.h>
#include <ESPmDNS.h> //Part of version 1.0.4 ESP32 Board Manager install
#include <ESP8266FtpServer.h>  //https://github.com/nailbuster/esp8266FTPServer  -->Needed for ftp transfers
#include <HTTPClient.h>   //Part of version 1.0.4 ESP32 Board Manager install  ----> Used for Domain Web Interace
#include <WiFiUdp.h>  //1.0.4 ESP32 Board Manager install
#include <sys/time.h>  // struct timeval --> Needed to sync time
#include <time.h>   // time() ctime() --> Needed to sync time
#include "FS.h"
#include "SPIFFS.h"
#include "Update.h"  //1.0.4 ESP32 Board Manager install
#include <ThingSpeak.h>   //https://github.com/mathworks/thingspeak-arduino . Get it using the Library Manager
#include <TinyGPS++.h> //http://arduiniana.org/libraries/tinygpsplus/ Used for GPS parsing
#include <Wire.h>    //Part of version 1.0.4 ESP32 Board Manager install  -----> Used for I2C protocol
#include <Ticker.h>  //Part of version 1.0.4 ESP32 Board Manager install  -----> Used for watchdog ISR
//#include <LiquidCrystal_I2C.h>   //https://github.com/esp8266/Basic/tree/master/libraries/LiquidCrystal optional
#include "variableInput.h"  //Packaged with project download.  Provides editing options; without having to search 2000+ lines of code.

// Replace with your network details
//const char* host;

// Replace with your network credentials (STATION)
const char* ssid = "#######";
const char* password = "######";

// ACCESS POINT credentials
const char* ssidAP = "ESP32-Access-Point";
const char* passwordAP = "123456789";

#import "index1.h"  //Weather HTML; do not remove

#import "index2.h"  //SdBrowse HTML; do not remove

#import "index3.h"  //Graphs HTML; do not remove

#import "index4.h"  //Restarts server; do not remove

#import "index5.h"  //Contactus.HTML; do not remove

#import "index6.h"  //Read File HTML; do not remove

#import "index7.h"  //Video feed HTML; do not remove

IPAddress ipREMOTE;

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Combo #2 = BOARD_ID 2, ESP32 Receiver #3 = BOARD_ID 3etc)
#define BOARD_ID 2

//Wi-Fi channel (must match the gateway wi-fi channel as an access point)
#define CHAN_AP 5

///Are we currently connected?
boolean connected = false;

///////////////////////////////////////////////
WiFiUDP udp;
// local port to listen for UDP packets
//const int udpPort = 1337;
char incomingPacket[255];
char replyPacket[] = "Hi there! Got the message :-)";
//const char * udpAddress1;
//const char * udpAddress2;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message1 {
  int id;
  float temp;
  float heat;
  float hum;
  float dew;
  float press;
  unsigned int readingId;
} struct_message;

struct_message1 incomingReadings;

float temperature;
float heatId;
float humd;
float dewPt;
float currentPressure;



//////////////////////////   Sender part /////////////////////

//MAC Address of that receives incoming relay status
//uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t broadcastAddress[] = {0x30, 0xAE, 0xA4, 0xDF, 0xB3, 0x6C};

//Structure to send data
//Must match the receiver structure
typedef struct struct_message2 {
    int batteryRelay;
} struct_message2;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message2 incomingReadings2;

int relay;

//incomingReadings2 relay status
int incomingRelay;

esp_now_peer_info_t peerInfo;

// Variable to store if sending data was successful
String success;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

////////////////////////////////// Receiver part ////////////////////////

JSONVar board;

AsyncWebServer serverAsync(PORT);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.println("");
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);

  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));

  board["id"] = incomingReadings.id;
  board["temperature"] = incomingReadings.temp;
  board["heatindex"] = incomingReadings.heat;
  board["humidity"] = incomingReadings.hum;
  board["dewpoint"] = incomingReadings.dew;
  board["pressure"] = incomingReadings.press;
  board["readingId"] = (String)incomingReadings.readingId;
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  Serial.printf("t value: %4.2f \n", incomingReadings.temp);
  Serial.printf("i value: %4.2f \n", incomingReadings.heat);
  Serial.printf("h value: %4.2f \n", incomingReadings.hum);
  Serial.printf("d value: %4.2f \n", incomingReadings.dew);
  Serial.printf("p value: %4.2f \n", incomingReadings.press);
  Serial.printf("readingId value: %d \n", incomingReadings.readingId);
  Serial.println();

  temperature = incomingReadings.temp;
  heatId = incomingReadings.heat;
  humd = incomingReadings.hum;
  dewPt = incomingReadings.dew;
  currentPressure = incomingReadings.press;
  
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP-NOW DASHBOARD</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #bebebe; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>ESP-NOW DASHBOARD</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #1 - TEMPERATURE</h4><p><span class="reading"><span id="t1"></span> &deg;F</span></p><p class="packet">Reading ID: <span id="rt1"></span></p>
      </div>
      <div class="card heatindex">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #1- HEATINDEX</h4><p><span class="reading"><span id="i1"></span> &deg;F</span></p><p class="packet">Reading ID: <span id="ri1"></span></p>
      <div>
      <div class="card humidity">
        <h4><i class="fas fa-tint"></i> BOARD #1 - HUMIDITY</h4><p><span class="reading"><span id="h1"></span> &percnt;</span></p><p class="packet">Reading ID: <span id="rh1"></span></p>
      </div>
      <div class="card dewpoint">
        <h4><i class="fas fa-tint"></i> BOARD #1 - DEWPOINT</h4><p><span class="reading"><span id="d1"></span> &deg;F</span></p><p class="packet">Reading ID: <span id="rd1"></span></p>
      </div>
      <div class="card pressure">
        <h4><i class="fas fa-tint"></i> BOARD #1 - PRESSURE</h4><p><span class="reading"><span id="p1"></span> inHg</span></p><p class="packet">Reading ID: <span id="rp1"></span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');

 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);

 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);

 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);
  var obj = JSON.parse(e.data);
  console.log(obj);
  document.getElementById("t"+obj.id).innerHTML = obj.temperature.toFixed(2);
  document.getElementById("i"+obj.id).innerHTML = obj.heatindex.toFixed(2);
  document.getElementById("h"+obj.id).innerHTML = obj.humidity.toFixed(2);
  document.getElementById("d"+obj.id).innerHTML = obj.dewpoint.toFixed(2);
  document.getElementById("p"+obj.id).innerHTML = obj.pressure.toFixed(2);
  document.getElementById("rt"+obj.id).innerHTML = obj.readingId;
  document.getElementById("ri"+obj.id).innerHTML = obj.readingId;
  document.getElementById("rh"+obj.id).innerHTML = obj.readingId;
  document.getElementById("rd"+obj.id).innerHTML = obj.readingId;
  document.getElementById("rp"+obj.id).innerHTML = obj.readingId;
 }, false);
 }
</script>
</body>
</html>)rawliteral";

////////////////////////////////////////////////////////

#define TZ "EST+5EDT,M3.2.0/2,M11.1.0/2"

////////////////////////////////////////////////

WiFiClient client;

////////////////////////// Web Server /////////////////
//WiFiServer server(PORT);
///////////////////////////////////////////////////////

////////////////////////// FTP Server /////////////////
FtpServer ftpSrv;
///////////////////////////////////////////////////////

//////////////////  OTA Support ////////////////////////////////////

//const char* http_username = """"
//const char* http_password = "";

//flag to use from web update to reboot the ESP
bool shouldReboot = false;
int logon;

void onRequest(AsyncWebServerRequest *request)
{
  //Handle Unknown Request
  request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  //Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  //Handle upload
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  //Handle WebSocket event
}

//////////////////////////////// End OEA Support //////////////////////

static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

const double Home_LAT = 88.888888;                      // Your Home Latitude --edit with your data
const double Home_LNG = 88.888888;                      // Your Home Longitude --edit with your data
const char* WiFi_hostname = "esp32";

TinyGPSPlus gps;

Ticker secondTick;

Ticker onceTicker;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

volatile int watchdogCounter;
int totalwatchdogCounter;

void IRAM_ATTR ISRwatchdog()
{

  portENTER_CRITICAL_ISR(&mux);
  watchdogCounter++;
  delay(100);
  portEXIT_CRITICAL_ISR(&mux);

}

int DOW, MONTH, DATE, YEAR, HOUR, MINUTE, SECOND;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

char strftime_buf[64];

String dtStamp(strftime_buf);

String lastUpdate;

unsigned long delayTime;

/* Two "independant" timed events */
const long eventTime_1 = 5000; //gps
const long eventTime_2 = 1000;   //boot
const long eventTime_3 = 45 * 1000;
const long eventTime_4 = 5 * 60 * 1000;
const long eventTime_5 = 60 * 1000; //in ms

/* When did they start the race? */
unsigned long previousTime_1 = 0;
unsigned long previousTime_2 = 0;
unsigned long previousTime_3 = 0;
unsigned long previousTime_4 = 0;
unsigned long previousTime_5 = 0;


int lc = 0;
time_t tnow = 0;

int count = 0;

int i;

int error = 0;
int flag = 0;
int wait = 0;

int started;   //Used to tell if Server has started

//use I2Cscanner to find LCD display address, in this case 3F   //https://github.com/todbot/arduino-i2c-scanner/
//LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

//#define sonalert 9  // pin for Piezo buzzer

#define online 19  //pin for online LED indicator

float pastPressure;  //Previous pressure reading used to find pressure change difference.
float milliBars;   //Barometric pressure in millibars
float difference;   //change in barometric pressure drop; greater than .020 inches of mercury.

//long int id = 1;  //Increments record number

char* filelist[12];

String logging;

char *filename;
char str[] = {0};

String fileRead;
String fn;
String uncfn;
String urlPath; 

char MyBuffer[17];

String PATH;

//String publicIP;   //in-place of xxx.xxx.xxx.xxx put your Public IP address inside quotes

//define LISTEN_PORT;  // in-place of yyyy put your listening port number
// The HTTP protocol uses port 80 by default.

/*
  This is the ThingSpeak channel number for the MathwWorks weather station
  https://thingspeak.com/channels/YourChannelNumber.  It senses a number of things and puts them in the eight
  field of the channel:

  Field 1 - Temperature (Degrees C )
  Field 2 - Humidity (%RH)
  Field 3 - Barometric Pressure (hpa)
  Field 4 - Dewpoint
*/


/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */


//Calibrate rain bucket here
//Rectangle raingauge from Sparkfun.com weather sensors
//float rain_bucket_mm = 0.011*25.4;//Each dump is 0.011" of water
//DAVISNET Rain Collector 2
//float rain_bucket_mm = 0.01*25.4;//Each dump is 0.01" of water  //Convert inch to millmeter (0.01 * 25.4)

// volatiles are subject to modification by IRQs
//volatile unsigned long raintime, rainlast, raininterval, rain, Rainindtime, Rainindlast;  // For Rain
//int addr=0;

#define eeprom_size 512

String eepromstring = "0.00";

//for loop
//int i;

unsigned long lastSecond, last5Minutes;
float lastPulseCount;
int currentPulseCount;
float rain5min;
float rainFall;
float rainHour;
float rainDay;
float daysRain;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//Interrupt routines (these are called by the hardware interrupts, not by the main code)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define FIVEMINUTES (300*1000L)
#define REEDPIN 34   //was 32 Touch pin
#define REEDINTERRUPT 0

volatile int pulseCount_ISR = 0;



void IRAM_ATTR reedSwitch_ISR()
{
  static unsigned long lastReedSwitchTime;
  // debounce for a quarter second = max. 4 counts per second
  if (labs(millis() - lastReedSwitchTime) > 250)
  {
    portENTER_CRITICAL_ISR(&mux);
    pulseCount_ISR++;

    lastReedSwitchTime = millis();
    portEXIT_CRITICAL_ISR(&mux);
  }

}

void setup(void)
{

  Serial.begin(9600);

  WiFi.persistent( false ); // for time saving

  // Connecting to local WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_AP_STA);
  WiFi.config(ip, gateway, subnet, dns);
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

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = CHAN_AP;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  started = 0;   //Server started

  while (!Serial);

  Serial.println("");
  Serial.println("Version 1.0 'ESP32_Receiver_Combo_wip.ino'  05/19/2022 @ 06:36 EDT");
  Serial.println("Please wait; for network connection...");
  Serial.println("");

  wifi_Start();

  Wire.begin(21, 22);

  pinMode(online, OUTPUT);  //Set pinMode to OUTPUT for online LED

  ///////////////////////// FTP /////////////////////////////////
  //FTP Setup, ensure SPIFFS is started before ftp;
  ////////////////////////////////////////////////////////////////
#ifdef ESP32       //esp32 we send true to format spiffs if cannot mount
  if (SPIFFS.begin(true))
  {
#elif defined ESP8266
  if (SPIFFS.begin())

#endif
    Serial.println("SPIFFS opened!");
    Serial.println("");
    ftpSrv.begin(ftpUser, ftpPassword);    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)

  }
  /////////////////////// End FTP//////////////////////////////

   serverAsync.on("/dash", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  events.onConnect([](AsyncEventSourceClient * client) { 
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());

  }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });

  serverAsync.addHandler(&events);   
  
  serverAsync.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/FAVICON";
    //accessLog();
    if (! flag == 1)
    {
      request->send(SPIFFS, "/favicon.png", "image/png");

    }
    //end();
  });

  serverAsync.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {

    PATH = "/";
    accessLog();

    ipREMOTE = request->client()->remoteIP();

    if (! flag == 1)
    {
      request->send_P(200, PSTR("text/html"), HTML1, processor1);
    }
    end();
  });

  serverAsync.on("/Weather", HTTP_GET, [](AsyncWebServerRequest * request)
  {

    PATH = "/Weather";
    accessLog();

    ipREMOTE = request->client()->remoteIP();

    if (! flag == 1)
    {
      request->send_P(200, PSTR("text/html"), HTML1, processor1);
    }
    end();
  });

  serverAsync.on("/SdBrowse", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/SdBrowse";
    accessLog();
    if (! flag == 1)
    {
      request->send_P(200, PSTR("text/html"), HTML2, processor2);

    }
    end();
  });

  serverAsync.on("/Graphs", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/Graphs";
    accessLog();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML3, processor3);
    response->addHeader("Server", "ESP Async Web Server");
    if (! flag == 1)
    {
      request->send(response);

    }
    end();
  });

  serverAsync.on("/get-file", HTTP_GET, [](AsyncWebServerRequest *request){
  PATH = fn;
  accessLog();
  if (! flag == 1)
  {

      request->send(SPIFFS, fn, "text/txt");

  }
  end();

  });  
  
  serverAsync.on("/Show", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    
    if (! flag == 1)
    {
      
      request->send_P(200, PSTR("text/html"), HTML6, processor6);
      
    }
    //end();
  });
  

  serverAsync.on("/RTSP", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/RTSP";
    accessLog();
    relayOn();  //Switch relay ON
    onceTicker.once_ms(120000, relayOff);  //Function relayOff sends ESP-Now message to turn relay OFF
    if (! flag == 1)
    {
      
      request->send_P(200, PSTR("text/html"), HTML7, processor7);
      
    }

    end();

    

  });

  serverAsync.on("/Contactus", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/Contactus";
    accessLog();
    if (! flag == 1)
    {
      request->send_P(200, PSTR("text/html"), HTML5, processor5);

    }
    end();
  });

  /*
       serverAsync.on("/ACCESS.TXT", HTTP_GET, [](AsyncWebServerRequest * request)
       {
            PATH = "/ACCESS";
            accessLog();
            AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML3);
            response->addHeader("Server","ESP Async Web Server");
            if(! flag == 1)
            {
                 request->send(SPIFFS, "/ACCESS610.TXT");

            }
            end();
       });
  */

  serverAsync.on("/RESTART", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/RESTART";
    accessLog();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML4, processor4);
    response->addHeader("Server", "ESP Async Web Server");
    if (! flag == 1)
    {
      request->send(response);

    }
   
    end();

    ESP.restart();

  });

  ///////////////////// OTA Support //////////////////////

  //attach.AsyncWebSocket
  ws.onEvent(onEvent);
  serverAsync.addHandler(&ws);

  // attach AsyncEventSource
  serverAsync.addHandler(&events);

  // respond to GET requests on URL /heap
  serverAsync.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  // upload a file to /upload
  serverAsync.on("/upload", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    request->send(200);
  }, onUpload);

  // send a file when /index is requested
  serverAsync.on("/index", HTTP_ANY, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.htm");
  });

  // HTTP basic authentication
  serverAsync.on("/login", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/login";
    accessLog();
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    request->send(200, "text/plain", "Login Success; upload firmware!");
    logon = 1;
    end();
  });

  // Simple Firmware Update Form
  serverAsync.on("/update", HTTP_GET, [](AsyncWebServerRequest * request)
  {

    PATH = "/update";
    accessLog();
    if (logon == 1)
    {
      request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
      logon = 0;
      end();
    }
    else
    {
      request->send(404); //Sends 404 File Not Found
      logon = 0;
      end();
    }


  });

  serverAsync.on("/update", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    shouldReboot = !Update.hasError();

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final)
  {

    if (!index)
    {
      Serial.printf("Update Start: %s\n", filename.c_str());
      //Update.runAsync(true);
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
      {
        Update.printError(Serial);
      }
    }
    if (!Update.hasError())
    {
      if (Update.write(data, len) != len)
      {
        Update.printError(Serial);
        end();
      }
    }
    if (final)
    {
      if (Update.end(true))
      {
        Serial.printf("Update Success: %uB\n", index + len);
        end();
      }
      else
      {
        Update.printError(Serial);
      }
    }


  });


  // attach filesystem root at URL /fs
  //serverAsync.serveStatic("/fs", SPIFFS, "/");

  // Catch-All Handlers
  // Any request that can not find a Handler that canHandle it
  // ends in the callbacks below.
  serverAsync.onNotFound(onRequest);
  serverAsync.onFileUpload(onUpload);
  serverAsync.onRequestBody(onBody);

  //serverAsync.begin();

  ///////////////////////// End OTA Support /////////////////////////////

  serverAsync.onNotFound(notFound);

  secondTick.attach(1, ISRwatchdog);  //watchdog  ISR triggers every 1 second

  configTime(0, 0, udpAddress1, udpAddress2);
  setenv("TZ", "EST+5EDT,M3.2.0/2,M11.1.0/2", 3);   // this sets TZ to Indianapolis, Indiana
  tzset();

  /*
       Serial.print("wait for first valid timestamp ");

       while (time(nullptr) < 100000ul)
       {
            Serial.print(".");
            delay(5000);
       }
  */

  pinMode(REEDPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REEDPIN), reedSwitch_ISR, FALLING);

  // initialize EEPROM with predefined size
  EEPROM.begin(eeprom_size);

  //RESET EEPROM CONTENT - ONLY EXECUTE ONE TIME - AFTER COMMENT

  /*

       Uncomment to 'clear'.eeprom values.

       Serial.println("CLEAR ");
       eepromClear();
       Serial.println("SET ");
       eepromSet("rain5min", "0.00");
       eepromSet("rainDay", "0.00");
       eepromSet("rainHour", "0.00");
       Serial.println("LIST ");
       Serial.println(eepromList());
  */

  //END - RESET EEPROM CONTENT - ONLY EXECUTE ONE TIME - AFTER COMMENT
  //eepromClear();

  //GET STORED RAINCOUNT IN EEPROM
  Serial.println("");
  Serial.println("GET EEPROM --Setup");
  eepromstring = eepromGet("rainDay");
  rainDay = eepromstring.toFloat();
  Serial.print("RAINDAY VALUE FROM EEPROM: ");
  Serial.println(rainDay);

  eepromstring = eepromGet("rainHour");
  rainHour = eepromstring.toFloat();
  Serial.print("RAINHOUR VALUE FROM EEPROM: ");
  Serial.println(rainHour);

  eepromstring = eepromGet("rain5min");
  rain5min = eepromstring.toFloat();
  Serial.print("rain5min VALUE FROM EEPROM: ");
  Serial.println(rain5min);
  Serial.println("");
  //END - GET STORED RAINCOUNT IN EEPROM

  //SPIFFS.format();

  //lcdDisplay();      //   LCD 1602 Display function --used for inital display

  ThingSpeak.begin(client);

  //delay(30 * 1000);  //Used to test reconnect WiFi routine.  Will produce one entry for each disconnect in "WIFI.TXT."

  //WiFi.disconnect();  //Used to test reconnect WiFi routine.  Will produce one entry for eac disconnect in "WIFI.TXT."

  //delay(50 * 1000);  //Uncomment to test watchdog

  //Serial.println("Delay elapsed");

  started = 1;

}

void loop()
{ 

  /* Updates frequently */
  unsigned long currentTime = millis();

  //udp only send data when connected
  if (connected)
  {
    //Send a packet
    udp.beginPacket(udpAddress1, udpPort);
    udp.printf("Seconds since boot: %u", millis() / 1000);
    udp.endPacket();
  }

  if (WiFi.status() != WL_CONNECTED)
  {

    wifi_Start();

    delay(10 * 1000);   //wait 10 seconds before writing

    getDateTime();

    //Open a "WIFI.TXT" for appended writing.   Client access ip address logged.
    File logFile = SPIFFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }
    else
    {
      logFile.print("Reconnected WiFi:  ");

      logFile.println(dtStamp);

      started = 1;
    }


  }

  delay(1);

  if (started == 1)
  {

    delay(2000);

    getDateTime();

    digitalWrite(online, HIGH);  //Indicates when Server is "Ready" for Browser Requests.
    delay(3000);
    digitalWrite(online, LOW);

    // Open a "log.txt" for appended writing
    File log = SPIFFS.open("/SERVER.TXT", "a");

    if (!log)
    {
      Serial.println("file 'SERVER.TXT' open failed");
    }

    log.print("Restarted Server:  ");
    log.print("  ");
    log.print(dtStamp);
    log.println("");
    log.close();

    if (((currentTime - previousTime_1 > eventTime_1) > 5000) && gps.charsProcessed() < 10);
    
    previousTime_1 = currentTime;

    started = 0;

  }

  if (watchdogCounter > 45)
  {

    totalwatchdogCounter++;

    Serial.println("watchdog Triggered");

    Serial.print("Watchdog Event has occurred. Total number: ");
    Serial.println(watchdogCounter / 80);

    // Open a "log.txt" for appended writing
    File log = SPIFFS.open("/WATCHDOG.TXT", "a");

    if (!log)
    {
      Serial.println("file 'WATCHDOG.TXT' open failed");
    }

    getDateTime();

    log.print("Watchdog Restart:  ");
    log.print("  ");
    log.print(dtStamp);
    log.println("");
    log.close();

    Serial.println("Watchdog Restart  " + dtStamp);

    WiFi.disconnect();

    ESP.restart();

  }
  
  watchdogCounter = 0;  //Resets the watchdogCount

  ///////////////////////////////////////////////////// FTP ///////////////////////////////////
  for (int x = 1; x < 5000; x++)
  {
    ftpSrv.handleFTP();
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////

  ///////////////////////// OTA Support ///////////////////////

  if (shouldReboot)
  {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }

  static char temp[128];
  sprintf(temp, "Seconds since boot: %u", (currentTime - previousTime_2));   // / 1000);   //time since boot
  events.send(temp, "time"); //send event "time"

  previousTime_2 = currentTime;

  //////////////////// End OTA Support /////////////////////////

  // each second read and reset pulseCount_ISR
  //if (millis() - lastSecond >= 1000)
  if ( currentTime - previousTime_3 >= eventTime_3) {   //rain gauge interupt

    lastSecond += 1000;
    portENTER_CRITICAL(&mux);
    currentPulseCount += pulseCount_ISR; // add to current counter
    pulseCount_ISR = 0; // reset ISR counter
    rainFall = currentPulseCount * .047; //Amout of rain in one bucket dump.
    portEXIT_CRITICAL(&mux);

    previousTime_3 = currentTime;

  }

  // each 5 minutes save data to another counter
  //if (millis()-last5Minutes>=FIVEMINUTES)
  if (currentTime - previousTime_4 >= eventTime_4) {   //rain gauge

    rain5min = rainFall;
    rainHour = rainHour + rainFall;  //accumulaing 5 minute rainfall for 1 hour then reset -->rainHour Rainfall
    rainDay = rainDay + rainFall;  //aacumulating 1 day rainfall
    last5Minutes += FIVEMINUTES; // remember the time
    lastPulseCount += currentPulseCount; // add to last period Counter
    currentPulseCount = 0;; // reset counter for current period

    previousTime_4 = currentTime;

  }

  for (int x = 1; x < 5000; x++)
  {
    ftpSrv.handleFTP();

  }

  getDateTime();

  //Serial.println(dtStamp);

  //Executes every 1 Minute Routine.
  if ((MINUTE % 1 == 0) && (SECOND == 0))
  {

    delay(2000);

    static unsigned long lastEventTime = millis();
    static const unsigned long EVENT_INTERVAL_MS = 60 * 1000;  //20000 = 20 seconds
    //if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    if (currentTime - previousTime_5 >= eventTime_5) {   //sender

      events.send("ping", NULL, millis());
      lastEventTime = millis();

      /*
        Serial.println(temp);
        Serial.println(incomingReadings.hum);
        Serial.println(incomingReadings.press);
      */
      previousTime_5 = currentTime;

    }
  }

  //Executes 5 Minute Routine.
  if ((MINUTE % 5 == 0) && (SECOND == 0))
  {

    delay(2000);

    Serial.println("");
    Serial.println("Five Minute routine");
    Serial.println(dtStamp);



    //STORE RAINCOUNT IN EEPROM
    Serial.println("SET EEPROM rainHour");
    eepromstring = String(rainHour, 2);
    eepromSet("rainHour", eepromstring);
    //END - STORE RAINCOUNT IN EEPROM

    //STORE RAINCOUNT IN EEPROM
    Serial.println("SET EEPROM rainDay");
    eepromstring = String(rainDay, 2);
    eepromSet("rainDay", eepromstring);
    //END - STORE RAINCOUNT IN EEPROM

    //STORE RAINCOUNT IN EEPROM
    Serial.println("SET EEPROM rain5min");
    eepromstring = String(rain5min, 2);
    eepromSet("rain5min", eepromstring);
    //END - STORE RAINCOUNT IN EEPROM
    Serial.println("");

    rainFall = 0;
    rain5min = 0;

    flag = 0;

  }

  //Executes 15 Minute routine and one Five Minute Rountine.
  if ((MINUTE % 15 == 0) && (SECOND == 0))
  {

    Serial.println("");
    Serial.println("Fifthteen minute routine");
    Serial.println(dtStamp);

    delayTime = 1000;

    readings();

    lastUpdate = dtStamp;   //store dtstamp for use on dynamic web page
    updateDifference();  //Get Barometric Pressure difference
    logtoSD();   //Output to LittleFS  --Log to LittleFS on 15 minute interval.
    delay(10);  //Be sure there is enough LittleFS write time
    pastPressure = currentPressure;  //Store last 15 MINUTE, currentPressure
    webInterface();
    speak();


  }

  if ((MINUTE == 59) && (SECOND == 59)) // one hour counter
  {
    rainHour = 0;
    rain5min = 0;
    rainFall = 0;
  }
  
  if ((HOUR == 23) && (MINUTE == 58) && (SECOND == 58)) //24 Hour; clear ALL.
  {
    fileStore();
  
    rain5min = 0;
    rainFall = 0;
    rainHour = 0;
    rainDay = 0;
    daysRain = 0;
  
  }
  
}

String processor1(const String& var)
{

  //index1.h

  if (var == F("LASTUPDATE"))
    return lastUpdate;

  if (var == F("GPSLAT"))
    return String(gpslat, 5);

  if (var == F("GPSLNG"))
    return String(gpslng, 5);

  if (var == F("GPSALT"))
    return String(gpsalt, 1);

  if (var == F("TEMP"))
    return String(temperature, 1);

  if (var == F("HEATINDEX"))
    return String(heatId, 1);

  if (var == F("HUM"))
    return String(humd);

  if (var == F("DEWPOINT"))
    return String(dewPt, 1);

  if (var == F("PRESSURE"))
    return String((currentPressure), 3);

  if (var == F("DIF"))
    return String((difference), 3);


  if (var == F("RAINDAY"))
    return String(rainDay);

  if (var == F("RAINHOUR"))
    return String(rainHour);

  if (var == F("RAINFALL"))
    return String(rainFall);

  if (var == F("DTSTAMP"))
    return dtStamp;

  if (var == F("LINK"))
    return linkAddress;

  if (var == F("CLIENTIP"))
    return ipREMOTE.toString().c_str();

  return String();

}

String processor2(const String& var)
{

  //index2.h

  String str;

  File root = SPIFFS.open("/");

  File file = root.openNextFile();

  while (file)
  {

    if (strncmp(file.name(), "/LOG", 4) == 0)
    {
      str += "<a href=\"";
      str += file.name();
      str += "\">";
      str += file.name();
      str += "</a>";
      str += "    ";
      str += file.size();
      str += "<br>\r\n";

    }

    file = root.openNextFile();
  }

  root.close();

  root.rewindDirectory();

  if (var == F("URLLINK"))
    return  str;

  if (var == F("LINK"))
    return linkAddress;

  if (var == F("FILENAME"))
    return  file.name();

  return String();

}

String processor3(const String& var)
{

  //index3.h

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

String processor4(const String& var)
{

  //index4.h

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

String processor5(const String& var)
{

  //index5.h

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

String processor6(const String& var)
{

  //index6.h

  if (var == F("FN"))
    return fn;

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

String processor7(const String& var)
{

  //index7.h

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

void accessLog()
{

  digitalWrite(online, HIGH);  //turn on online LED indicator

  getDateTime();

  String ip1String = "10.0.0.146";   //Host ip address
  String ip2String = ipREMOTE.toString().c_str();   //client remote IP address

  Serial.println("");
  Serial.println("Client connected:  " + dtStamp);
  Serial.print("Client IP:  ");
  Serial.println(ip2String);
  Serial.print("Path:  ");
  Serial.println(PATH);
  Serial.println(F("Processing request"));

  //Open a "access.txt" for appended writing.   Client access ip address logged.
  File logFile = SPIFFS.open(Restricted, FILE_APPEND);

  if (!logFile)
  {
    Serial.println("File 'ACCESS.TXT'failed to open");
  }

  if ((ip1String == ip2String) || (ip2String == "0.0.0.0") || (ip2String == "(IP unset)"))
  {

    //Serial.println("HOST IP Address match");
    logFile.close();

  }
  else
  {

    Serial.println("Log client ip address");

    logFile.print("Accessed:  ");
    logFile.print(dtStamp);
    logFile.print(" -- Client IP:  ");
    logFile.print(ip2String);
    logFile.print(" -- ");
    logFile.print("Path:  ");
    logFile.print(PATH);
    logFile.println("");
    logFile.close();

  }

}

void beep(unsigned char delayms)
{

  //     wait for a delayms ms
  //     digitalWrite(sonalert, HIGH);
  //     delayTime = 3000;
  //     digitalWrite(sonalert, LOW);

}

//----------------------------- EEPROM -------------- Muhammad Haroon --------------------------------

void eepromSet(String name, String value)
{
  Serial.println("eepromSet");

  String list = eepromDelete(name);
  String nameValue = "&" + name + "=" + value;
  //Serial.println(list);
  //Serial.println(nameValue);
  list += nameValue;
  for (int i = 0; i < list.length(); ++i)
  {
    EEPROM.write(i, list.charAt(i));
  }
  EEPROM.commit();
  Serial.print(name);
  Serial.print(":");
  Serial.println(value);

  delayTime = 1000;

}


String eepromDelete(String name)
{
  Serial.println("eepromDelete");

  int nameOfValue;
  String currentName = "";
  String currentValue = "";
  int foundIt = 0;
  char letter;
  String newList = "";
  for (int i = 0; i < 512; ++i)
  {
    letter = char(EEPROM.read(i));
    if (letter == '\n')
    {
      if (foundIt == 1)
      {
      }
      else if (newList.length() > 0)
      {
        newList += "=" + currentValue;
      }
      break;
    }
    else if (letter == '&')
    {
      nameOfValue = 0;
      currentName = "";
      if (foundIt == 1)
      {
        foundIt = 0;
      }
      else if (newList.length() > 0)
      {
        newList += "=" + currentValue;
      }

    }
    else if (letter == '=')
    {
      if (currentName == name)
      {
        foundIt = 1;
      }
      else
      {
        foundIt = 0;
        newList += "&" + currentName;
      }
      nameOfValue = 1;
      currentValue = "";
    }
    else
    {
      if (nameOfValue == 0)
      {
        currentName += letter;
      }
      else
      {
        currentValue += letter;
      }
    }
  }

  for (int i = 0; i < 512; ++i)
  {
    EEPROM.write(i, '\n');
  }
  EEPROM.commit();
  for (int i = 0; i < newList.length(); ++i)
  {
    EEPROM.write(i, newList.charAt(i));
  }
  EEPROM.commit();
  Serial.println(name);
  Serial.println(newList);
  return newList;
}

void eepromClear()
{
  Serial.println("eepromClear");
  for (int i = 0; i < 512; ++i)
  {
    EEPROM.write(i, '\n');
  }
}

String eepromList()
{
  Serial.println("eepromList");
  char letter;
  String list = "";
  for (int i = 0; i < 512; ++i)
  {
    letter = char(EEPROM.read(i));
    if (letter == '\n')
    {
      break;
    }
    else
    {
      list += letter;
    }
  }
  Serial.println(list);
  return list;
}

String eepromGet(String name)
{
  Serial.println("eepromGet");

  int nameOfValue;
  String currentName = "";
  String currentValue = "";
  int foundIt = 0;
  String value = "";
  char letter;
  for (int i = 0; i < 512; ++i)
  {
    letter = char(EEPROM.read(i));
    if (letter == '\n')
    {
      if (foundIt == 1)
      {
        value = currentValue;
      }
      break;
    }
    else if (letter == '&')
    {
      nameOfValue = 0;
      currentName = "";
      if (foundIt == 1)
      {
        value = currentValue;
        break;
      }
    }
    else if (letter == '=')
    {
      if (currentName == name)
      {
        foundIt = 1;
      }
      else
      {
      }
      nameOfValue = 1;
      currentValue = "";
    }
    else
    {
      if (nameOfValue == 0)
      {
        currentName += letter;
      }
      else
      {
        currentValue += letter;
      }
    }
  }
  Serial.print(name);
  Serial.print(":");
  Serial.println(value);
  return value;
}

void seteeprom()
{

  eepromstring = String(rainDay, 2);
  eepromSet("rainDay", eepromstring);

  rain5min = 0;

  eepromstring = String(rainHour, 2);
  eepromSet("rainHour", eepromstring);

  eepromstring = String(rain5min, 2);
  eepromSet("rain5min", eepromstring);


  //END - STORE RAINCOUNT IN EEPROM

}

//------------------------------- end EEPROM --------- Muhammad Haroon -------------------------------------

void end()
{

  delay(1000);

  digitalWrite(online, LOW);   //turn-off online LED indicator

  getDateTime();

  Serial.println("Client closed:  " + dtStamp);

}

void fileStore()   //If Midnight, rename "LOGXXYYZZ.TXT" to ("log" + month + day + ".txt") and create new, empty "LOGXXYYZZ.TXT"
{

  int temp;
  String Date;
  String Month;

  temp = (DATE);
  if (temp < 10)
  {
    Date = ("0" + (String)temp);
  }
  else
  {
    Date = (String)temp;
  }

  temp = (MONTH);
  if (temp < 10)
  {
    Month = ("0" + (String)temp);
  }
  else
  {
    Month = (String)temp;
  }

  String logname;  //file format /LOGxxyyzzzz.txt
  logname = "/LOG";
  logname += Month;   ///logname += Clock.getDate();
  logname += Date; ////logname += Clock.getMonth(Century);
  logname += YEAR;
  logname += ".TXT";

  //Open file for appended writing
  File log = SPIFFS.open(logname.c_str(), "a");

  if (!log)
  {
    Serial.println("file open failed");
  }

}

//////////////////////////////////
//Get Date and Time
//////////////////////////////////
String getDateTime()
{
  struct tm *ti;

  tnow = time(nullptr) + 1;
  //strftime(strftime_buf, sizeof(strftime_buf), "%c", localtime(&tnow));
  ti = localtime(&tnow);
  DOW = ti->tm_wday;
  YEAR = ti->tm_year + 1900;
  MONTH = ti->tm_mon + 1;
  DATE = ti->tm_mday;
  HOUR  = ti->tm_hour;
  MINUTE  = ti->tm_min;
  SECOND = ti->tm_sec;

  strftime(strftime_buf, sizeof(strftime_buf), "%a , %m/%d/%Y , %H:%M:%S %Z", localtime(&tnow));
  dtStamp = strftime_buf;
  return (dtStamp);

}


//////////////////////////////////////////////////////
//Pressure difference for fifthteen minute interval
/////////////////////////////////////////////////////
float updateDifference()  //Pressure difference for fifthteen minute interval
{


  //Function to find difference in Barometric Pressure
  //First loop pass pastPressure and currentPressure are equal resulting in an incorrect difference result.  Output "...Processing"
  //Future loop passes difference results are correct

  difference = currentPressure - pastPressure;  //This will be pressure from this pass thru loop, pressure1 will be new pressure reading next loop pass
  if (difference == currentPressure)
  {
    difference = 0;
  }
  return (difference); //Barometric pressure change in inches of Mecury

}

//Write to SPIFSS
void logtoSD()   //Output to SPIFFS every fifthteen minutes
{

  getDateTime();

  int tempy;
  String Date;
  String Month;

  tempy = (DATE);
  if (tempy < 10)
  {
    Date = ("0" + (String)tempy);
  }
  else
  {
    Date = (String)tempy;
  }

  tempy = (MONTH);
  if (tempy < 10)
  {
    Month = ("0" + (String)tempy);
  }
  else
  {
    Month = (String)tempy;
  }

  String logname;
  logname = "/LOG";
  logname += Month;   ///logname += Clock.getDate();
  logname += Date; ////logname += Clock.getMonth(Century);
  logname += YEAR;
  logname += ".TXT";

  // Open a "log.txt" for appended writing
  //File log = SPIFFS.open(logname.c_str(), FILE_APPEND);
  File log = SPIFFS.open(logname.c_str(), FILE_APPEND);

  if (!log)
  {
    Serial.println("file 'LOG.TXT' open failed");
  }

  delay(500);

  //log.print(id);
  //log.print(" , ");
  log.print("Lat: ");
  log.print(gpslat, 5);
  log.print(" , ");
  log.print("Long: ");
  log.print(gpslng, 5);
  log.print(" , ");
  log.print(lastUpdate);
  log.print(" , ");
  
  log.print("Temp:  ");
  log.print(temperature, 1);
  log.print(" F. , ");
  
  log.print("HeatId:  ");
  log.print(heatId, 1);
  log.print(" F. , ");

  log.print("Hum:  ");
  log.print(humd, 1);
  log.print(" % , ");
  
  log.print("DewPt:  ");
  log.print(dewPt, 1);
  log.print(" % , ");  

  log.print("Press:  ");
  log.print(currentPressure, 3);
  log.print(" inHg. ");
  log.print(" , ");

  if (pastPressure == currentPressure)
  {
    log.print("0.000");
    log.print(" Difference ");
    log.print(" ,");
  }
  else
  {
    log.print((difference), 3);
    log.print(" Difference ");
    log.print(", ");
  }

  log.print(" Day ");
  log.print(rainDay, 2);
  log.print(" ,");

  log.print(" Hour ");
  log.print(rainHour, 2);
  log.print(" , ");

  log.print(" Five Minute ");
  log.print(rain5min, 2);
  log.print(" , ");

  log.print("Elev:  ");
  log.print(gpsalt, 0);
  log.print(" feet. ");
  log.println();

  //Increment Record ID number
  //id++;

  Serial.println("");

  Serial.println("Data written to  " + logname + "  " + dtStamp);

  log.close();

  pastPressure = currentPressure;

  if (abs(difference) >= .020) //After testing and observations of Data; raised from .010 to .020 inches of Mecury
  {
    // Open a "Differ.txt" for appended writing --records Barometric Pressure change difference and time stamps
    File diffFile = SPIFFS.open("DIFFER.TXT", FILE_APPEND);

    if (!diffFile)
    {
      Serial.println("file 'DIFFER.TXT' open failed");
    }

    Serial.println("");
    Serial.print("Difference greater than .020 inches of Mecury ,  ");
    Serial.print(difference, 3);
    Serial.print("  ,");
    Serial.print(dtStamp);

    diffFile.println("");
    diffFile.print("Difference greater than .020 inches of Mecury,  ");
    diffFile.print(difference, 3);
    diffFile.print("  ,");
    diffFile.print(dtStamp);
    diffFile.close();

    beep(50);  //Duration of Sonalert tone

  }
}

//readFile  --AsyncWebServer version with much help from Pavel
String notFound(AsyncWebServerRequest *request)
{

  digitalWrite(online, HIGH);   //turn-on online LED indicator

  if (! request->url().endsWith(F(".TXT")))
  {
    request->send(404);
  }
  else
  {
    if (request->url().endsWith(F(".TXT")))
    {
      //.endsWith(F(".txt")))

      // here comes some mambo-jambo to extract the filename from request->url()
      int fnsstart = request->url().lastIndexOf('/');

      fn = request->url().substring(fnsstart);

      uncfn = fn.substring(1);

      urlPath = linkAddress + "/" + uncfn;            

    }    

  }
  
  request->redirect("/Show");

  digitalWrite(online, LOW);   //turn-off online LED indicator 

  return fn; 

}

void readings()
{

  temperature = incomingReadings.temp;
  heatId = incomingReadings.heat;
  humd = incomingReadings.hum;
  dewPt = incomingReadings.dew;
  currentPressure = incomingReadings.press;

}

void relayOn()
{
  
  relay = 1;

  struct_message2 incomingReadings2;
  incomingReadings2.batteryRelay = relay;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &incomingReadings2, sizeof(incomingReadings2));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");

  } 

  relay = 3;   //any value other than 0 or 1

} 

void relayOff()
{
  
  relay = 0;

  struct_message2 incomingReadings2;
  incomingReadings2.batteryRelay = relay;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &incomingReadings2, sizeof(incomingReadings2));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");

  } 

  relay = 3;   //any value other than 0 or 1


}

//ThingSpeak.com --Graphing and iftmes
void speak()
{

  char t_buffered1[14];
  dtostrf(temperature, 7, 1, t_buffered1);

  char t_buffered2[14];
  dtostrf(humd, 7, 1, t_buffered2);

  char t_buffered3[14];
  dtostrf(currentPressure, 7, 1, t_buffered3);

  char t_buffered4[14];
  dtostrf(dewPt, 7, 1, t_buffered4);

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  ThingSpeak.setField(1, t_buffered1);  //Temperature
  ThingSpeak.setField(2, t_buffered2);  //Humidity
  ThingSpeak.setField(3, t_buffered3);  //Barometric Pressure
  ThingSpeak.setField(4, t_buffered4);  //Dew Point F.

  // Write the fields that you've set all at once.
  //ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  getDateTime();

  Serial.println("");
  Serial.println("Sent data to Thingspeak.com  " + dtStamp + "\n");

}

//Hosted Domain, web page -code sends data for Dynamic web page every 15 Minutes
void webInterface()
{

  char glat[10]; // Buffer big enough for 9-character float
  dtostrf(gpslat, 9, 4, glat); // Leave room for too large numbers!

  char glng[10]; // Buffer big enough for 9-character float
  dtostrf(gpslng, 9, 4, glng); // Leave room for too large numbers!

  char fahr[7];// Buffer big enough for 9-character float
  dtostrf(temperature, 6, 1, fahr); // Leave room for too large numbers!

  char heatindex[7];// Buffer big enough for 9-character float
  dtostrf(heatId, 6, 1, heatindex); // Leave room for too large numbers!

  char humidity[7]; // Buffer big enough for 9-character float
  dtostrf(humd, 6, 1, humidity); // Leave room for too large numbers!

  char dewpoint[7]; // Buffer big enough for 9-character float
  dtostrf(dewPt, 6, 1, dewpoint); // Leave room for too large numbers!

  char barometric[9]; // Buffer big enough for 7-character float
  dtostrf(currentPressure, 8, 3, barometric); // Leave room for too large numbers!

  char diff[9]; // Buffer big enough for 7-character float
  dtostrf(difference, 8, 3, diff); // Leave room for too large numbers!

  char rain5[10]; // Buffer big enough for 9-character float
  dtostrf(rain5min, 6, 3, rain5); // Leave room for too large numbers!

  char rain60[10]; // Buffer big enough for 9-character float
  dtostrf(rainHour, 6, 3, rain60); // Leave room for too large numbers!

  char rain24[10]; // Buffer big enough for 9-character float
  dtostrf(rainDay, 6, 3, rain24); // Leave room for too large numbers!

  char alt[9]; // Buffer big enough for 9-character float
  dtostrf(843, 8, 1, alt); // Leave room for too large numbers!

  String data = "&last="                  +  (String)lastUpdate

                + "&glat="                +  glat

                + "&glng="                +  glng

                + "&fahr="                +  fahr

                + "&heatindex="           +  heatindex

                + "&humidity="            +  humidity

                + "&dewpoint="            +  dewpoint

                + "&barometric="          +  barometric

                + "&diff="                +  diff

                + "&rain5="               +  rain5

                + "&rain60="              +  rain60

                + "&rain24="              +  rain24

                + "&alt="                 +  alt;

  if (WiFi.status() == WL_CONNECTED)
  {
    //Check WiFi connection status

    HTTPClient http;    //Declare object of class HTTPClient

    http.begin(sendData);      //Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header

    int httpCode = http.POST(data);   //Send the request
    String payload = http.getString();                  //Get the response payload

    if (httpCode == 200)
    {
      Serial.print("");
      Serial.print("HttpCode:  ");
      Serial.print(httpCode);   //Print HTTP return code
      Serial.print("  Data echoed back from Hosted website  " );
      Serial.println("");
      Serial.println(payload);    //Print payload response

      http.end();  //Close HTTPClient http
    }
    else
    {
      Serial.print("");
      Serial.print("HttpCode:  ");
      Serial.print(httpCode);   //Print HTTP return code
      Serial.print("  Domain website data update failed.  ");
      Serial.println("");

      http.end();   //Close HTTPClient http
    }

  }
  else
  {

    Serial.println("Error in WiFi connection");

  }

}

/////////////////////////////////////
//wiFi Start-up and connection code
////////////////////////////////////
void wifi_Start()
{

  //WiFi.disconnect();

  //delayTime = (10 * 1000);

  //WiFi.mode(WIFI_OFF);

  WiFi.mode(WIFI_AP_STA);

  Serial.println();
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  // We start by connecting to WiFi Station
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  delay(1000);

  //setting the static addresses in function "wifi_Start
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
  IPAddress dns;

  WiFi.config(ip, gateway, subnet, dns);

  WiFi.begin(ssid, password);


  Serial.println("Web server running. Waiting for the ESP32 IP...");

  // Printing the ESP IP address
  Serial.print("Server IP:  ");
  Serial.println(WiFi.localIP());
  Serial.print("Port:  ");
  Serial.println(LISTEN_PORT);
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());
  Serial.println("\n");

  delayTime = 500;

  WiFi.waitForConnectResult();

  serverAsync.begin();

  // Set device as an access point
  WiFi.softAP(ssidAP, passwordAP, CHAN_AP, false);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  //esp_now_register_recv_cb(1);

  Serial.printf("Connection result: %d\n", WiFi.waitForConnectResult());

  serverAsync.begin();


  if (WiFi.waitForConnectResult() != 3)
  {
    delay(3000);
    wifi_Start();

  }

}
