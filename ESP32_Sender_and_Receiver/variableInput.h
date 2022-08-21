//
//    Only edit ######## entries with your information; make no other changes!!!!
//   variableInput.h library
//   William M. Lucid   01/12/2022 @ 15:26 EST  
// 

//   Edit "ESP32_Sender_and_Receiver.ino" lines 1453 and 1454 to enable email alerts and sms alerts  --sms alerts maybe late arriving!
//   Edit "ESP32_Sender_and_Receiver.ino" line 1458.  Comment out this line to use US-100 ultrasonic sensor.
//
//   Router wireless channel is set to channel 5; this  Change router to match or change variable CHAN_AP to your router wireless channel.
//   Edit "ESP32_Sender_and_Receiver with your data; lines 281, 1422, and 1423 --Used with sending SMS alertsand sending email alerts.
//   CHAN_AP can be found in "ESP32_Sender_and_Receiver.ino." line 102; also edit in "ESP32_Recever_Only.ino" line 56   WiFi.softAP(ssidAP,passwordAP,5);

// Replace with your network details   
const char * host  = "esp8266";

// Replace with your network details
const char * ssid = "######";
const char * password = "########";

// ACCESS POINT credentials
const char* ssidAP = "ESP32-Access-Point";
const char* passwordAP = "123456789";

//Settings pertain to NTP
const int udpPort = 1337;
//NTP Time Servers
const char * udpAddress1 = "us.pool.ntp.org";
const char * udpAddress2 = "time.nist.gov";

//publicIP accessiable over Internet with Port Forwarding; know the risks!!!
//WAN IP Address.  Or use LAN IP Address --same as server ip; no Internet access. 
//#define publicIP  70,225,18,250  //Part of href link for "GET" requests
#define publicIP  ###,###,###,### //Part of href link for "GET" requests

String LISTEN_PORT = "####"; //Part of href link for "GET" requests
String linkAddress = "##########:####";  //publicIP and PORT for URL link

String ip1String = "##############";  //Host ip address  

//int PORT 
int PORT = ####;  //Web Server port

//Co-Orinates from GPS/cell phone (Your location)
float gpslat = ##>#####;
float gpslng = ###>#####;
String alt = "########";

//Graphing requires "FREE" "ThingSpeak.com" account..  
//Enter "ThingSpeak.com" data here....
//Example data; enter yout account data..
//unsigned long int myChannelNumber = 1623233; 
//const char * myWriteAPIKey = "85P57IEWRWZQLQTH";

unsigned long int myChannelNumber = #######; 
const char * myWriteAPIKey = "###########";

//Server settings
IPAddress ip {10,0,0,115};
IPAddress subnet {255,255,255,0};
IPAddress gateway {10,0,0,1};
IPAddress dns {10,0,0,1};

//FTP Credentials
const char * ftpUser = "#######";
const char * ftpPassword = "#########";
 
//Restricted Access
const char* Restricted = "/#########";  //Can be any filename.  
//Will be used for "GET" request path to pull up client ip list.

///////////////////////////////////////////////////////////
//   "pulicIP/LISTEN_PORT/reset" wiill restart the server
///////////////////////////////////////////////////////////

///////////////// OTA Support //////////////////////////

const char* http_username = "#####";
const char* http_password = "######";

// xx.xx.xx.xx:yyyy/login will log in; this will allow updating firmware using:
// xx.xx.xx.xx:yyyy/update
//
// xx.xx.xx.xx being publicIP and yyyy being PORT.
//
///////////////////////////////////////////////////////
