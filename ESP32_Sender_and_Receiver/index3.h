//index2.h
const char HTML3[] PROGMEM = R"====(
<!DOCTYPE HTML>
<html>

<head>
    <title>Graphs</title>
</head>

<body>
  <br>
	<h2>Graphed Weather Observations</h2>
	<br>
  <frameset rows="30%,70%" cols="33%,34%">
  <iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/290421/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Temperature&type=line&xaxis=Date+and+Time&yaxis=Temperature++++F."></iframe>  
  <iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/290421/charts/2?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Relative+Humidity&type=line&xaxis=Date+and+Time&yaxis=Relative+Humidity+Percent"></iframe>
  </frameset>
  <br><br>
  <frameset rows="30%,70%" cols="33%,34%">
  <iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/290421/charts/3?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Barometric+Pressure&type=line&xaxis=Date+and+Time&yaxis=Pressure+++inHg"></iframe>
  <iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/290421/charts/4?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Dew+Point&type=line&xaxis=Date+and+Time&yaxis=Dew+Point+++++F."></iframe>    	
  </frameset>
 	<br><br>
  <a href=http://%LINK%/Weather >Home</a>
  <br>
</body>

</html>
)====";
