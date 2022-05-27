//
//   variableInput.h library
//   William Lucid   5/26/2022   @ 11:31 EDT 
// 
//   Replace with your creditals anything with "#'s"
//


// Replace with your network details  
//const char * host  = "####";

// Replace with your network details
//const char * ssid = "####";
//const char * password = "#####";

//Settings pertain to NTP
const int udpPort = 1337;
//NTP Time Servers
const char * udpAddress1 = "us.pool.ntp.org";
const char * udpAddress2 = "time.nist.gov";

//publicIP accessiable over Internet with Port Forwarding; know the risks!!!
//WAN IP Address.  Or use LAN IP Address --same as server ip; no Internet access. 
#define publicIP  "###.###.###.###"  //Part of href link for "GET" requests
String LISTEN_PORT = "8030"; //Part of href link for "GET" requests

String linkAddress = "###.###.###.###:8030";  //publicIP and PORT for URL link

String ip1String = "10.0.0.146";  //Host ip address  

int PORT = 8030;  //Web Server port

//Graphing requires "FREE" "ThingSpeak.com" account..  
//Enter "ThingSpeak.com" data here....
//Example data; enter yout account data..
unsigned long int myChannelNumber = #####; 
const char * myWriteAPIKey = "#####";

//Server settings
#define ip {10,0,0,110}
#define subnet {255,255,255,0}
#define gateway {10,0,0,1}
#define dns {10,0,0,1}

float gpslat = ########; 
float gpslng = #########;

#define gpsalt ####


//webInterface --send Data to Domain, hosted web site
const char * sendData = "###########/collectdata2.php";

//FTP Credentials
const char * ftpUser = "#########";
const char * ftpPassword = "##########";
 
//Restricted Access
const char* Restricted = "/###########";  //Can be any filename.  
//Will be used for "GET" request path to pull up client ip list.

///////////////////////////////////////////////////////////
//   "pulicIP/LISTEN_PORT/reset" wiill restart the server
///////////////////////////////////////////////////////////

///////////////// OTA Support //////////////////////////

const char* http_username = "######";
const char* http_password = "######";

// xx.xx.xx.xx:yyyy/login will log in; this will allow updating firmware using:
// xx.xx.xx.xx:yyyy/update
//
// xx.xx.xx.xx being publicIP and yyyy being PORT.
//
///////////////////////////////////////////////////////
