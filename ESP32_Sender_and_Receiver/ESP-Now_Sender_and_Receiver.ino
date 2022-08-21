///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//    Updated            Version  2.0 Aux_Sump_Monitor__ESP_Now.ino  08/15/2022 10:57 EDT  Developed by William  M. Lucid
//               
//                       Added WiFi Client Event logging of Web server starts, Brownouts, Watchdog, WiFi disconnects, WiFi reconnects.
//                       
//                       Environmental Calculations for Dewpoint, Heatindex, and Sea level Barometric Pressure.
//
//                       Developing AsyncWebServer 11/07/2019; modifying with fileRead and not found function.  Adding wifi log of reconnects.
//
//                       Portion of NTP time code was developed from code provided by schufti  --of ESP8266Community
//
//                       Original listFiles and readFile functions by martinayotte of ESP8266 Community Forum.  Function readFile modified by RichardS of ESP8266 Community Forum; for ESP32.
//
//                       Thank you Pavel for your help with modifying readFile function; enabling use with AsyncWebServer!
//
//                       Time keeping functions uses NTP Time.
//
//                       GPS and rain gauge code developed by Muhammad Haroon.  Thank you Muhammad.
//
//                       Previous projects:  https://github.com/tech500
//
//                       Project is Open-Source, requires one BME280 breakout board, a NEO m8n GPS Module, and a "HiLetgo ESP-WROOM-32 ESP32 ESP-32S Development Board"
//
//                       http://monitorone.ddns.net  Project web page  --Servered from ESP32.
//
//
//                       Note:  Uses ESP32 core by ESP32 Community, version 2.0.4; from "Arduino IDE, Board Manager."   Arduino IDE; use Board:  "Node32s" for the "HiLetGo" ESP32 Board.
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*

Building new version to start and stop aux sump pit pump based on level of primary sump pit.

08/12/2022 @ 01:04 EDI

Developed by William M. Lucid 
ab9nq.william@gmail.com

Untested on AT&T Network Gateway 1/12/2022  @ 15:38 EST
Tested on Xfinity Network 1/12/2022 @ 15:00 EST status:  working

This copy is for use by Joe Leggins
to monitor his Sump Pump.

Program uses ultrasonic sensing to measure distance of water from top of sump pit.

Features:

1.  Ultrasonic distance to water measuring,
2.  Log file of measurements at 15 minute intervals.
3.  Web interface display last update of water distance to top of sump pit.
4.  Graph of distance to top and time of data point.
5.  FTP for file maintence; should it be needed.
6.  Automatic deletion of log files.  Can be daily of weekly
7.  OTA Over-the-air firmware updates.

*/


// ********************************************************************************
// ********************************************************************************
//
//   See library downloads for each library license.
//
// ********************************************************************************
// ********************************************************************************


#include <arduino.h>
#include "EMailSender.h"   //https://github.com/xreef/EMailSender
#include <esp_now.h>
//#include "EEPROM.h"  //Part of version 2.0.4 ESP32 Board Manager install
#include <WiFi.h>   //Part of version 2.0.4 ESP32 Board Manager install
#include <WiFiUdp.h>  //2.0.4 ESP32 Board Manager install
//#include <DNSServer.h>
//#include <WebServer.h>
//#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <HTTPClient.h>  //Part of version 2.0.4 ESP32 Board Manager install
#include <AsyncTCP.h>  //https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h>  //https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPmDNS.h> //Part of version 2.0.4 ESP32 Board Manager install
#include <FTPServer.h>  //https://github.com/dplasa/FTPClientServer
//#include <HTTPClient.h>   //Part of version 2.0.4 ESP32 Board Manager install  ----> Used for Domain Web Interace
#include <sys/time.h>  // struct timeval --> Needed to sync time
#include <time.h>   // time() ctime() --> Needed to sync time
#include <FS.h>
#include <LittleFS.h>
#include <Update.h>  //2.0.4 ESP32 Board Manager install
#include <ThingSpeak.h>   //https://github.com/mathworks/thingspeak-arduino . Get it using the Library Manager
//#include <BME280I2C.h>   //Use the Arduino Library Manager, get BME280 by Tyler Glenn
//Addition information on this library:  https://github.com/finitespace/BME280
//#include <EnvironmentCalculations.h>  //Use the Arduino Library Manager, get BME280 by Tyler Glenn
#include <Wire.h>    //Part of version 2.0.4 ESP32 Board Manager install  -----> Used for I2C protocol
#include <Ticker.h>  //Part of version 2.0.4 ESP32 Board Manager install  -----> Used for watchdog ISR
#include <rom/rtc.h>
//#include <LiquidCrystal_I2C.h>   //https://github.com/esp8266/Basic/tree/master/libraries/LiquidCrystal optional
#include "variableInput.h"  //Packaged with project download.  Provides editing options; without having to search 2000+ lines of code.

// Replace with your network details
//const char* host;

// Replace with your network details
//const char* ssid;
//const char* password;

#import "index1.h"   //HTML; do not remove

#import "index2.h"   //HTML; do not remove

#import "index3.h"   //HTML; do not remove

#import "index4.h"   //HTML; do not remove

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Combo #2 = BOARD_ID 2, ESP32 Receiver #3 = BOARD_ID 3etc)
#define BOARD_ID 2

//Wi-Fi channel (must match the gateway wi-fi channel as an access point)
#define CHAN_AP 5

int relay = 0;

//MAC Address of that receives incoming relay status
//uint8_t broadcastAddress1[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//uint8_t broadcastAddress1[] = {0x30, 0xAE, 0xA4, 0xDF, 0xB3, 0x6C};   //ESP32 Dev Board:  MAC: 30:AE:A4:DF:B3:6C
uint8_t broadcastAddress1[] = {0xE0, 0xE2, 0xE6, 0x9B, 0x86, 0x70};   //Adafruit Feather

//Structure to send data
//Must match the receiver structure
typedef struct struct_message2 {
    int batteryRelay;
} struct_message2;

// Create a struct_message called incomingReadings2 to hold relay status.
struct_message2 remoteRelay;

esp_now_peer_info_t peerInfo;

char strftime_buf[64];

String dtStamp(strftime_buf);

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");   
  getDateTime();
  Serial.println(dtStamp);

}

int fileCount = 0;

unsigned long int a;

char* fileList[30];

int connect = 0;
int disconnect = 0;
int count = 1;
int counter = 0;
int brownout = 0;
int softReset = 0;
int powerOn = 0;

void WiFiEvent(WiFiEvent_t event) {
  //Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      //Serial.println("Connected to access point");
      connect = 1;
      disconnect = 0;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      //Serial.println("Disconnected from WiFi access point");
      disconnect = 1;
      if(event == 7){
        disconnect = 0;
      }
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("\nObtained IP address: ");
      Serial.println(WiFi.localIP());
      break;
    default: break;
  }
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

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

#define TZ "EST+5EDT,M3.2.0/2,M11.1.0/2"

////////////////////////////////////////////////

WiFiClient client;

////////////////////////// Web Server /////////////////
//WiFiServer server(PORT);
///////////////////////////////////////////////////////

//////////////////////////   Sender part /////////////////////

///////////////////////////// FTP Server /////////////////
FTPServer ftpSrv(LittleFS);
///////////////////////////////////////////////////////

////////////////////////// AsyncWebServer ////////////
AsyncWebServer serverAsync(PORT);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)
//////////////////////////////////////////////////////

//////////////////  OTA Support ////////////////////////////////////

//const char* http_username = "____";
//const char* http_password = "_____";

//flag to use from web update to reboot the ESP
bool shouldReboot = false;
int logon;

uint8_t connection_state = 0;
uint16_t reconnect_interval = 10000;

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

//EMailSender emailSend("esp8266.sump@gmail.com", "Mysumpalert$");
EMailSender emailSend("lucidw.esp8266@gmail.com", "vfwtnrizswrwbavg");

static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

const char* WiFi_hostname = "esp32";

Ticker secondTick;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

volatile int watchdogCounter;
volatile int watchDog = 0;

void IRAM_ATTR ISRwatchdog()
{

  portENTER_CRITICAL_ISR(&mux);
  
  watchdogCounter++;
  
  if (watchdogCounter >= 75) 
  {

    watchDog = 1;

  }

  portEXIT_CRITICAL_ISR(&mux);

}

//unsigned long start, finished, elapsed, runTime;
float start, finished, elapsed, runTime;

int DOW, MONTH, DATE, YEAR, HOUR, MINUTE, SECOND;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

String lastUpdate;
String status;
String value;

unsigned long delayTime;

int lc = 0;
time_t tnow = 0;

int i;
int countFiles;
int error = 0;
int flag = 0;
int wait = 0;

//use I2Cscanner to find LCD display address, in this case 3F   //https://github.com/todbot/arduino-i2c-scanner/
//LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

//#define sonalert 9  // pin for Piezo buzzer

#define online 19  //pin for online LED indicator

//int switchRelay;  //switch camera power on or off to conserve battery

//long int id = 1;  //Increments record number

char* filelist[12];

int counted;

String logging;

char *filename;
String fn;
String uncfn;
String urlPath; 

char str[] = {0};

String fileRead;

char MyBuffer[17];

String PATH;

int trigPin = 14;   //Orange wire
int echoPin = 12;   //White wire

int pingTravelTime;
float pingTravelDistance;
float distanceToTarget;
int dt = 50;

int reconnect;

String urgent ="";

int textRequest = 0;   //Limits SMS text to one text.  Requires manaul reset of Sump Pit Monitor.


//String publicIP;   //in-place of xxx.xxx.xxx.xxx put your Public IP address inside quotes

//define LISTEN_PORT;  // in-place of yyyy put your listening port number
// The HTTP protocol uses port 80 by default.

int pass = 2;  //relay OFF

void setup()
{

  Serial.begin(115200);

  while (!Serial) {}

  Serial.println("");
  Serial.println("\nVersion  2.0 Aux_Sump_Monitor__ESP_Now.ino  07:54 EDT");
  Serial.println("");

  if (rtc_get_reset_reason(0) == 1)  //VBAT_RESET --brownout restart
  {
  
    brownout = 1;

    Serial.println("Brownout reset previous boot");
      
  }
  
  if (rtc_get_reset_reason(0) == 12)  //SOFTWARE_RESET --watchdog restart 
  {
  
    softReset = 1;

    Serial.println("Software reset previous boot");
      
  }  
  
  Serial.println("\nConnecting to WiFi...");

  // Examples of different ways to register wifi events
  WiFi.onEvent(WiFiEvent);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFiEventId_t eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
      //Serial.print("WiFi lost connection. Reason: \n");
      //Serial.println(info.wifi_sta_disconnected.reason);
  }, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // Remove WiFi event
  //Serial.print("WiFi Event ID: ");
  ////Serial.println(eventID);
  //WiFi.removeEvent(eventID);

   WiFi.persistent( false ); // for time saving

  // Connecting to local WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.config(ip, gateway, subnet, dns);
  WiFi.softAP(ssidAP,passwordAP,5);
  WiFi.begin(ssid, password);
  delay(10);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());
 
  Serial.print("ESP32 IP on the WiFi network: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Server IP:  ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);

  // register peer
  peerInfo.channel = CHAN_AP;  
  peerInfo.encrypt = false;
  // register first peer  
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
   
  Wire.begin(21, 22);

  pinMode(25, OUTPUT);

  secondTick.attach(1, ISRwatchdog);  //watchdog ISR increase watchdogCounter by 1 every 1 second

  pinMode(online, OUTPUT);  //Set pinMode to OUTPUT for online LED

  if(LittleFS.begin(true))
  {

    Serial.println("LittleFS opened!");
    Serial.println("");
    ftpSrv.begin(ftpUser, ftpPassword); //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)

  }
  /////////////////////// End FTP//////////////////////////////


  serverAsync.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    PATH = "/FAVICON";
    //accessLog();
    if (! flag == 1)
    {
       request->send(LittleFS, "/favicon.ico", "image/png");

    }
    //end();
  });

  serverAsync.on("/sump", HTTP_GET, [](AsyncWebServerRequest * request)
  {

  PATH = "/sump";
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

  serverAsync.on("/graph", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      PATH = "/graph";
      accessLog();
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML3, processor3);
      response->addHeader("Server", "ESP Async Web Server");
      if (! flag == 1)
      {
        request->send(response);

      }
      end(); 
    });

  serverAsync.on("/Show", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      
      if (! flag == 1)
      {
        
        request->send_P(200, PSTR("text/html"), HTML4, processor4);
        
      }
      
    });

    serverAsync.on("/get-file", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, fn, "text/txt");
    PATH = fn;
    accessLog();
    end();

    });

    serverAsync.on("/redirect/internal", HTTP_GET, [](AsyncWebServerRequest *request){
      request->redirect("/Show");  //recevies HTML request to redirect
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
    serverAsync.on("/upload+-", HTTP_POST, [](AsyncWebServerRequest * request)
    {
      request->send(200);
    }, onUpload);

    // send a file when /index is requested
    serverAsync.on("/index", HTTP_ANY, [](AsyncWebServerRequest * request)
    {
      request->send(LittleFS, "/index.htm");
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
    
    serverAsync.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(401);
      logon = 0;
 });


  // attach filesystem root at URL /fs
  //serverAsync.serveStatic("/fs", LittleFS, "/");

  // Catch-All Handlers
  // Any request that can not find a Handler that canHandle it
  // ends in the callbacks below.
  serverAsync.onNotFound(onRequest);
  serverAsync.onFileUpload(onUpload);
  serverAsync.onRequestBody(onBody);
  
///////////////////////// End OTA Support /////////////////////////////

  serverAsync.onNotFound(notFound);  

  //LittleFS.format();

  //lcdDisplay();      //   LCD 1602 Display function --used for inital display

  //ThingSpeak.begin(client);

  //WiFi.disconnect();  //Used to test reconnect WiFi routine.  

  //delay(75 * 1000);  //Uncomment to test watchdog

  //Serial.println("Delay elapsed");
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  textRequest = 0;

  serverAsync.begin();

  //ultra();   //////////////////////////////////////////////////////////////////////// uncomment

  powerOn = 1;

  watchdogCounter = 0;
  
}

void loop() 
{

  //udp only send data when connected
  if (connected)
  {

    //Send a packet
    udp.beginPacket(udpAddress1, udpPort);
    udp.printf("Seconds since boot: %u", millis() / 1000);
    udp.endPacket();

  }

  digitalWrite(online, LOW);

  delay(1);

  if ((brownout == 1) || (softReset == 1))  
  {
        
    watchdogCounter = 0;  //Resets the watchdogCounter
    
    getDateTime();
    
    //Open a "WIFI.TXT" for appended writing.   Client access ip address logged.
    File logFile =LittleFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }
    
    if((brownout == 1) && (powerOn != 1))  
    {
      
      logFile.println("ESP32 Restart caused by Brownout Detection...");

      brownout = 0;

      connect = 1;

    }

    if((brownout == 1) && (powerOn == 1)) 
    {
      
      powerOn = 0;

      Serial.println("ESP32 Webserver Started...");
      
      logFile.println("ESP32 Webserver Started...");

      brownout = 0;

      connect = 1;
      
    }

    if (softReset == 1)
    {

      logFile.println("ESP32 Restart caused by Watchdog Event...");

      softReset = 0;

    }  

    logFile.close();
        
  }

  if(WiFi.status() == WL_NO_SSID_AVAIL )  //Prevent watchdog triggering if no wifi
  {

    watchdogCounter = 0;

  }

  if (connect == 1) 
  {

    //Serial.println("Connect:  " + (String)connect);

    delay(1000);
    
    configTime();

    //Open a "WIFI.TXT" for appended writing.   
    File logFile = LittleFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }

    logFile.print("WiFi Connected:       ");

    logFile.print(dtStamp);

    logFile.printf("   Connection result: %d\n", WiFi.waitForConnectResult());

    logFile.println("");
    
    logFile.close();

    Serial.println("Logged WiFi connection, timestamp\n");

    Serial.println("Ready...\n");

    watchdogCounter = 0;  //Resets the watchdogCounter

    connect = 0;
   
    counter = 1;      
    
  }
  
  if((disconnect == 1) && (counter == 1))
  {

    //Serial.println("Counter:  " + (String)counter);
    //Serial.println("Disconnect:  " + (String)disconnect);

    
    watchdogCounter = 0;  //Resets the watchdogCounter

    //Open "WIFI.TXT" for appended writing.   Client access ip address logged.
    File logFile = LittleFS.open("/WIFI.TXT", "a");

    if (!logFile)
    {
      Serial.println("File: '/WIFI.TXT' failed to open");
    }

    getDateTime(); 

    logFile.print("WiFi Disconnected:   ");  

    logFile.print(dtStamp);

    logFile.print("   Connection result: ");

    logFile.println(WiFi.waitForConnectResult());

    logFile.close();

    Serial.println("\nLogged WiFi disconnection, timestamp\n");
      
    watchdogCounter = 0;

    disconnect = 0;

    counter = 0;

  }

  if (watchDog == 1)
	{

		portENTER_CRITICAL(&mux);
		watchdogCounter--;
		portEXIT_CRITICAL(&mux);

    getDateTime();

		logWatchdog();
		
	}

  for (int x = 1; x < 5000; x++)
  {
    ftpSrv.handleFTP();
  }

  ///////////////////////// OTA Support ///////////////////////

  if (shouldReboot)
  {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
  static char temp[128];
  sprintf(temp, "Seconds since boot: %u", millis() / 1000);
  events.send(temp, "time"); //send event "time"

  //////////////////// End OTA Support /////////////////////////

  getDateTime();

  //Executes every15 Minutes routine
  if((MINUTE % 2 == 0) && (SECOND == 15))
  {

    delay(1000);

    flag = 1;

    Serial.println("15 Minute Routine\n");
    Serial.println(dtStamp);
    Serial.println("");
    lastUpdate = dtStamp;   //Store dtstamp for use on dynamic web page
    ultra();
    logtoSD();   //Output to LittleFS  --Log to LittleFS on 15 minute interval.
    value = distanceToTarget;
    delay(100);
    thingSpeak();
    
    textRequest = 0;  

    flag = 0;

  }
   
  getDateTime();	    

  if ((HOUR == 23) && (MINUTE == 59) && (SECOND == 0)) //Start new kog file..
  {

    if (DOW == 6)
    {
      
       listDel();  //Removes "LOG*.TXT" files from Sunday thru Saturday. do on before Midnight Saturday.

    }
    
    delayTime = 1000;

  }
  
  watchdogCounter = 0;  //reset watchdogCounter

  powerOn = 0;

}

String processor1(const String& var)
{

     //index1.h
     
     if (var == F("TOP"))
          return String(distanceToTarget, 1);

     if (var == F("ALERT"))
          return urgent;

     if (var == F("DATE"))
          return lastUpdate;

     if (var == F("STATUS"))
          return status;

     if (var == F("LINK"))
          return linkAddress;

     if (var == F("CLIENTIP"))
          return ipREMOTE.toString().c_str();

     return String();

}

String processor2(const String& var)
{

	//index2.h

  String url;

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS failed to mount !");
  }

  File root = LittleFS.open("/");

  File file = root.openNextFile();

  while(file)
  {

        String file_name = file.name();

        if (file_name.startsWith("LOG"))
        {
          
          url += "<a href=\"";
          url += file.name();
          url += "\">";
          url += file.name();
          url += "</a>";
          url += "    ";
          url += file.size();
          url += "<br>\r\n";

        } 

        file = root.openNextFile();       

  }

  root.close();

  if (var == F("URLLINK"))
    return  url;

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

  if (var == F("FN"))
    return fn;

  if (var == F("LINK"))
    return linkAddress;

  return String();

}

void accessLog() 
{

  digitalWrite(online, HIGH);  //turn on online LED indicator

  getDateTime();

  String ip2String = ipREMOTE.toString().c_str();   //client remote IP address
  String returnedIP = ip2String;
  
  Serial.println("");
  Serial.println("Client connected:  " + dtStamp);
  Serial.print("Client IP:  ");
  Serial.println(returnedIP);
  Serial.print("Path:  ");
  Serial.println(PATH);
  Serial.println(F("Processing request"));

  //Open a "access.txt" for appended writing.   Client access ip address logged.
  File logFile =LittleFS.open(Restricted, FILE_APPEND);

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
    logFile.print(returnedIP);
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

void configTime()
{

  configTime(0, 0, udpAddress1, udpAddress2);
  setenv("TZ", "EST+5EDT,M3.2.0/2,M11.1.0/2", 3);   // Sets TZ to Indianapolis, Indiana
  tzset();

  Serial.print("wait for first valid timestamp");

  while (time(nullptr) < 100000ul)
  {
    Serial.print(".");
    delay(5000);
  }

  Serial.println("\nSystem Time set\n");

  getDateTime();

  Serial.println(dtStamp);

}

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
  logname += Month; ////logname += Clock.getMonth(Century);
  logname += Date;   ///logname += Clock.getDate();
  logname += YEAR;
  logname += ".TXT";

  //Open file for appended writing
  File log =LittleFS.open(logname.c_str(), FILE_APPEND);

  if (!log)
  {
    Serial.println("File open failed");
  }

}

void listDel()
{	
  
  a = 0;
  
  File root = LittleFS.open("/");

  File file = root.openNextFile();

  while(file) 
  {

    //String str = fileName();

		if(strncmp(file.name(), "LOG", 4) == 0)
    {

        i++;
	      counted = countFiles++;
        filelist[i] = strdup(file.name());
               
    }

    file = root.openNextFile();
    		
	}  

  while(countFiles > 0)
  {

    for(a = 1; a < countFiles + 1; a++)
    {

      LittleFS.remove(fileList[a]);
      Serial.print("Removed:  ");
      Serial.print(fileList[a]);
      Serial.println("");    
      
    }

    Serial.print("");
    Serial.println("No more files to remove.\n");
         
    fileCount = 0;         
   
  } 

  delay(1000);
     
}

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

void logtoSD()   //Output to LittleFS every fifthteen minutes
{


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
     File rec= LittleFS.open(logname.c_str(), "a");

     //For troubleshooting
     //Serial.println(logname.c_str());

     if (!rec)
     {
          Serial.println("LOG --rec open failed");
     }

     delay(500);

     rec.print(distanceToTarget);
     rec.print(" inches ");
     rec.print(" , ");
     rec.print(dtStamp);
     rec.print(urgent);
     rec.print(",");
     rec.print("  Aux. Pump:  ");
     rec.print(status);

     Serial.print("relay:  ");
     Serial.println(relay);

    if((relay == 0) && (runTime > 0 ))
    {  
      
      rec.print(",");
      rec.print("  Run time:  ");
      rec.print(runTime);
      rec.print("  Minute/s");

    }

    delay(250);

     
     rec.println();
    
     Serial.println("");
     Serial.print(distanceToTarget, 1);
     Serial.println(" inches; Data written to  " + logname + "  " + dtStamp);
     
     rec.close();

     runTime = 0;     

}

void logWatchdog()
{

  Serial.println("");
  Serial.println("Watchdog event triggered.");
  Serial.println("Watchdog Restart  " + dtStamp);

  ESP.restart();

}

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

void sendRequestURL()  //Triggers cellphone SMS alert and email alert.
{

     if(textRequest == 1)
     {

          EMailSender::EMailMessage message;
          message.subject = "Warning High Water!!!";
          message.message = "Urgent --Sump Pump alert --high water level!!";
          
          EMailSender::Response resp = emailSend.send("3173405675@vtext.com", message);
          emailSend.send("lucidw.esp8266@gmail.com", message);

          Serial.println("Sending status: "); 

          Serial.println(resp.status);
          Serial.println(resp.code);
          Serial.println(resp.desc);

     }

     textRequest = 0;

}

void ultra()    //Get distance in inches from the floor (top of Sump Pit.)
{

  delay(10);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  pingTravelTime = pulseIn(echoPin, HIGH);
  delay(25);
  pingTravelDistance = (pingTravelTime * 765.*5280.*12) / (3600.*1000000);
  distanceToTarget = (pingTravelDistance / 2);
  Serial.print("\nDistance to Target is: ");
  distanceToTarget = random(1,13);   //Used for testig; without us-100 ultrasonic sensor.
  Serial.print(distanceToTarget);
  Serial.println(" in.");

  digitalWrite(echoPin, HIGH);  

  delay(50);

  textRequest = 1;

  if(distanceToTarget <= 6 )   //Relay ON
  {

    if(pass == 1 )
    {

      Serial.print("pass:  ");
      Serial.println(pass);

      getDateTime();      
      
      relay = 1;

      start = millis();

      urgent = "   Alert";

      status = "ON ";

      //value = status;

      Serial.println("ON");

      remoteRelay.batteryRelay = relay;

      esp_err_t result = esp_now_send(broadcastAddress1, (uint8_t *) &remoteRelay, sizeof(remoteRelay));

      if (result == ESP_OK) {
        Serial.println("Sent with success... ");
      }
      else {
        Serial.println("Error sending the data");
      } 

      //sendRequestURL();   //Uncomment to enable email alerts and sms  messages.  --sms texts may be late arriving!

      pass = 2;

    }
    
  }
  
  if(distanceToTarget > 6)   //Relay OFF
  {

    if(pass == 2)  
    { 

      Serial.print("pass:  ");
      Serial.println(pass);

      
      getDateTime();
      
      relay = 0;

      finished = millis();
    
      elapsed = finished-start;

      runTime =  ((elapsed / 1000) / 60);

      urgent = " ";

      status = "OFF ";

      //value = status;

      Serial.println("OFF");   
  
      remoteRelay.batteryRelay = relay;      

      esp_err_t result = esp_now_send(broadcastAddress1, (uint8_t *) &remoteRelay, sizeof(remoteRelay));
      
      if (result == ESP_OK) {
        Serial.println("Sent with success... ");
      }
      else {
        Serial.println("Error sending the data");
      }
      
      delay(200);

      pass = 1;

    }
    
  }   

}

////////////////////////////////////////
//ThingSpeak.com --Graphing and iftmes
///////////////////////////////////////
void thingSpeak()
{  
  
  char distance[14];
  dtostrf(distanceToTarget, 10, 1, distance);

   // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  ThingSpeak.writeField(myChannelNumber, 1, distance, myWriteAPIKey); //Distsnce to top of dump pit.

  Serial.println("Sent data to Thingspeak.com  " + dtStamp + "\n");

  delay(2000);

}