# ESP-Now-with-Remote-Relay

Continuing ESP-Now previous project; added another ESP32 to remotely, switch relay.

Project is for monitoring the water level in a sump pit and switching via ESP-Now, a relay; for auxillary sump pump.  Features Data logging at 15 minute interval, web interface, file browser with file reader, graphing provided by "Thingspeak,"  Wifi Client Event logging of:  Web server starts, Brownout events, Watchdog events, WIFI connects, and WIFI disconnects with date/time stamping.  Additional features include FTP and OTA firmware updates.

Relay is emulated; easily, add your own relay.

https://github.com/Tech500/CameraRainGauge is the main portion of the Asyncwebserver in "ESP-Now-with-Remote-Relay."

Enter your data in "variableInput.h" input fields with "#########".  "variableInput.h" is found in the ESP32_Sender_and_Receiver folder.
