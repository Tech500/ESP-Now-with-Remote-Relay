<?php

/*
  ESP32: send data to your Domain 

  Uses POST command to send DHT data to a designated website
  The circuit:
  * BME280
  * Post to Domain 
  
  Original version of this code by Stephan Borsay
  
  https://www.hackster.io/detox/send-esp8266-data-to-your-webpage-no-at-commands-7ebfec
  
*/

date_default_timezone_set("America/Indianapolis"); 
$TimeStamp = date("Y-m-d   h:i:sa");

	if( $_REQUEST["last"] ||  $_REQUEST["glat"] ||  $_REQUEST["glng"] ||  $_REQUEST["fahr"] ||  $_REQUEST["heatindex"] ||  $_REQUEST["humidity"] ||  
	$_REQUEST["dewpoint"] || $_REQUEST["barometric"]  || $_REQUEST["diff"]  || $_REQUEST["rain5"] || $_REQUEST["rain60"]|| $_REQUEST["rain24"]|| $_REQUEST["alt"]) 
	{
		echo " The Last Update was: ". $_REQUEST['last']."" ;
		echo " The Latitude is: ". $_REQUEST['glat']. "  ";
		echo " The Logitude is: ". $_REQUEST['glng']. "  ";
		echo " The Temperature is: ". $_REQUEST['fahr']. " F ";
		echo " The Heatindex is: ". $_REQUEST['heatindex']. " F ";
		echo " The Humidity is: ". $_REQUEST['humidity']. " % ";
		echo " The Dewpoint is: ". $_REQUEST['dewpoint']. " F";
		echo " The Barometric Pressure is: ". $_REQUEST['barometric']. " inHG  ";
		echo " The Rainfall per 5 Minute is: ". $_REQUEST['rain5']. "   ";
		echo " The Rainfall per 60 Minutes is:  " . $_REQUEST['rain60']. "   ";
		echo " The Rainfall per 24 Hours is:  " . $_REQUEST['rain24']. "   ";
		echo " The Altitude is:  " . $_REQUEST['alt']. " Feet  ";
		
	}	  

	
$var1 = $_REQUEST['last']; 
$var2 = $_REQUEST['glat'];
$var3 = $_REQUEST['glng'];
$var4 = $_REQUEST['fahr'];
$var5 = $_REQUEST['heatindex'];
$var6 = $_REQUEST['humidity'];
$var7 = $_REQUEST['dewpoint'];
$var8 = $_REQUEST['barometric'];
$var9 = $_REQUEST['diff'];
$var10 = $_REQUEST['rain5'];
$var11 = $_REQUEST['rain60'];
$var12 = $_REQUEST['rain24'];
$var13 = $_REQUEST['alt'];

ob_start()
?>

<!DOCTYPE html>     
 <html>

 <head>
       
</head>
<style>  
    body {
        background-color: #561c0e;
    }

    p {
        color: ffffff;     
    }

    #header { 
        top: 250px;
	   text-align: center;
        line-height: 40%;
        font-size: 22px;        
    }
      
    .flex-container {
	  display: flex; 
       gap: 10px;	  
	  background-color: #561c0e;
	}

	.flex-container > div {    
	  background-color: #561c0e;
	  margin: 20px;
	  padding: 10px;
	  font-size: 18px;
	}

    #link {
        font-size: 28px;
        text-align: center;
        position: relative;
    }


</style>    

 <body>

     <div id='header' style='color: #ffffff'>       

         <p>CameraRainGauge.ino<br></p>
         <p>Indianapolis, IN, US.</p>


     </div>

     

    <div class="flex-container">

	   <div>
		  <p>
			 <p>Updated : <?php echo $var1 ?> </p>
			 <p>Latitude is : <?php echo $var2 ?> </p>
			 <p>Longitutde is : <?php echo $var3 ?> </p>
			 <p>Temperature is : <?php echo $var4 ?> F.</p>
			 <p>HeatIndex is : <?php echo $var5 ?> F.</p>
			 <p>Humidity is : <?php echo $var6 ?> % </p>
			 <p>Dewpoint is : <?php echo $var7 ?> F.</p.>

	   </div>

	   <div>
		  <p>
			 <p>Barometric Pressure is : <?php echo $var8 ?> inHg </p>
			 <p>Difference Changed : <?php echo $var9 ?> inHg </p>
			 <p>Rainfall per Five Minutes is : <?php echo $var10 ?> mm</p>
			 <p>Rainfall per Hour is : <?php echo $var11 ?> mm</p>
			 <p>Rainfall per Day is : <?php echo $var12 ?> mm</p>
			 <p>Elevation is : 843 Feet</p>
			 <p></p>
			 <p>
				
			 </p>
	   </div>
	   
    </div> 

    <div id='link' style='color: #ffffff'>

	  <a href='https://www.hackster.io/Techno500/data-logger-with-live-video-feed-e27f88?f=1&f=1' style='color: #ffffff'>Project on Hackster.io</a></p>
   
    </div>

    

     

     


 </body>

 </html>


<?php
file_put_contents('dataDisplayer2.html', ob_get_clean()); 
?>