//index1.h
const char HTML1[] PROGMEM = R"====(
<!DOCTYPE HTML>
<html>

<head>
    <title>Sump Pit</title>  
</head>
</head>

<body>
    <h1>Sump Pit Water Level</h1>
    <h2>Distance to top of Sump Pit:
    <br> 
    <br>%TOP% inches   %ALERT%
    <br>
    <br>Last Update:  %DATE%
    <br>
    <br>Auxillary Pump:  %STATUS%
    <br>        
    <br><br><a href=http://%LINK%/SdBrowse >File Browser</a>
    <br><a href=http://%LINK%/graph >Graph</a>
    <br><a href=http://%LINK%/WIFI.TXT >WiFi Log</a><br><br>
    <br> Client IP: %CLIENTIP%</h2><br><br>
</body>

</html>
)====";
