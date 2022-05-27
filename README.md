# ESP-Now-with-Remote-Relay --a work-in-progress

Continuing ESP-Now previous project; added another ESP32 to remotely, change relay from off to on and back to off.

Project uses one ESP32 for BME280 sensor readings from an ESP-Now sender configuration.  Second, ESP32 is a combination receiver and sender; hosts Async web server.  Using web server "GET" request for "Camera view" calls function "relayOn" to send ESP-Now struct_message to third, ESP32 turning on relay.  Third ESP32 configured as only a receiver to control a relay for completing battery power path powering a Wyse Cam v3 or to break battery power path.  Goal is to conserve battery power; switching on only during a two minute viewing window.  Ticker library instance onceTicker sets a two minute window; calling relayOff function; which sends struct message from second, ESP32 to the third, ESP32 turning relay off.
