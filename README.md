# ESP-Now-with-Remote-Relay --a work-in-progress

Continuing ESP-Now previous project; added another ESP32 to remotely, change relay from off to on and back to off.

Project uses one ESP32 for BME280 sensor readings from an ESP-Now sender configuration.  Second, ESP32 is a combination receiver and sender; hosts Async web server.  Using web server "GET" request for "Camera view" calls function "relayOn" to send ESP-Now struct_message to third, ESP32 turning on relay.  Third ESP32 configured as only a receiver to control a relay for completing battery power path powering a Wyse Cam v3 or to break battery power path.  Goal is to conserve battery power; switching on only during a two minute viewing window.  Ticker library instance onceTicker sets a two minute window; calling relayOff function; which sends struct message from second, ESP32 to the third, ESP32 turning relay off.

Relay is emulated; easily, add your own relay N.O or N.C.

https://github.com/Tech500/CameraRainGauge is the main portion of the Asyncwebserver in "ESP-Now-with-Remote-Relay."

"collectdata2.php" is used with hosting website on a domain website server; used with the Asyncwebserver, "webInterface" function.  Link in the file to Steohan Borsay's "Hackster.io" article "Send ESP8266 Data to Your Webpage - no AT Commands!" is about the use of "collectorData2.php" file.  Purpose of the php is to retrieve data from your webserver and build a new web page "on-the-fly" dynamically!

Enter your data in "variableInput.h" input fields with "#########".  "variableInput.h" is found in the ESP32_Sender_and_Receiver folder.
