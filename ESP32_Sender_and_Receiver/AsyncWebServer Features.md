

"ESP32_Sender_Receiver.ino" --Dynamic Web Server with NTP and RTSP Video!


1. NTP Server; used for 15 minute time interval, date-time stamping, and dayofweek (every 6th
day, "LOG.TXT" file gets renamed to keep file size manageable. Every Saturday "LOG.TXT" gets
renamed in the format "logxxyy" xx being the month and yy being the date; a new log.txt is
created after every file renaming.

2. Dynamic web page of current observations for Last update time and date, humidity, dew
point, temperature, heat index, barometric pressure.

3. Server files are listed as web links.

4. LOG.TXT file is appended every 15 minutes with the latest update; storing data from Dynamic
web page.

5. ACCESS.TXT restricted file that logs client IP address.

6. Optional, DIFFER.TXT stores the difference in Barometric Pressure for the last fifteen 
minutes. Only a difference of equal to or greater than .020 inches of Mercury are logged 
with difference, date and time.

7. URL file names and Menu links are only "GET" requests recognized.

8. Optional, Audible alert from Piezo electric buzzer when there is Barometric Pressure 
difference of .020 inches of Mercury. I am interested in sudden drop of Barometric Pressure in 
a 15 minute interval. Serve weather more likely with a sudden drop. Difference of .020 inches 
of Mercury point is set for my observations to log and sound audible alert; not based on any 
known value to be associated with serve weather --experimental.

9. Optional, Two-line LCD Display of Barometric Pressure, in both inches of Mercury and millibars.

10. Tempature, Humidity, Barometric Pressure, and Dew Point have four embedded "ThinkSpeak.com"
graphs on one web page. Graphs are created from Iframes provided by "ThingSpeak.com"

11. Free, "000webhost powered by HOSTINGER" is used for second website.

12. Two websites,one sketch:"ESP32_Sender_Receiver.ino" --Dynamic Web Server with NTP and 
RTSP Video!

 http://weather-3.ddns.net/Weather  Project web site servered from ESP32; has file browser and 
 video feed.

 https://observeredweather.000webhostapp.com   Project web site served by Free Hosting Service

13.  OTA, over-the-air firmware updates and FTP, file transfer protocol implemented.

14.  New:  "Camera View," main Menu option; provides RTSP, Wyse Cam v3 near real time, video feed.



