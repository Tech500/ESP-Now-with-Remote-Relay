//index6.h
const char HTML7[] PROGMEM = R"====(
<!DOCTYPE html>
<html>
<head>

  <meta charset="utf-8">  
  <title>Unreal Media Server WebRTC player</title>
  
  <script type="text/javascript" src="http://gc.kis.v2.scr.kaspersky-labs.com/FD126C42-EBFA-4E12-B309-BB3FDD723AC1/main.js?attr=5_kmXRo0UtYPkMmvi2pueMJ6TwZtFe75E64hAzwNX3T7zLrRHvxN1HY6Y5-TWOn1wOXmCU82BwjNFhbXOfXXoZzIOwXMUFzny9SKZ2ckHOwGLIDBjuLnuE1JrU5lRPY9" charset="UTF-8"></script><link rel="stylesheet" crossorigin="anonymous" href="http://gc.kis.v2.scr.kaspersky-labs.com/E3E8934C-235A-4B0E-825A-35A08381A191/abn/main.css?attr=aHR0cDovL3d3dy51bWVkaWFzZXJ2ZXIubmV0L3VtZWRpYXNlcnZlci9kZW1vaHRtbDVXZWJSVENwbGF5ZXIuaHRtbA"/><script src="http://www.umediaserver.net/umediaserver/webrtcadapter.js"></script>
  <script src="http://www.umediaserver.net/umediaserver/unrealwebrtcplayer.js"></script>

    <script type="text/javascript">

      var webrtcPlayer = null;

    </script>

</head>

<body>

  <div id="container" align="center">
  
    <br><br><br>
    Unreal Media Server WebRTC player<br><br>This player plays live near real time audio/video on any OS and mobile device, in all major browsers. This is original H264 video encoded by IP camera; server doesn't do any transcoding.
    <br><br>Wyse Cam v3; providing RTSP video feed.  Camera maybe offline; depending on battery discharge state.
     <!-- <A style="COLOR: darkgreen;" href="demohtml5WebRTCplayerSecure.html">Same page with signaling over secure WebSocket</A> -->
    <br><br><br><br><br><br><br><br>
        
    <video style="background-color:black" id="remoteVideo" width="800" height="450" autoplay playsinline controls muted></video>

    <script type="text/javascript">

        //Parameters:

        //1. ID of video element.
        //2. Alias of the live broadcast; video encoding supported: H264(AVC1), VP8, VP9; audio encoding supported: opus, G.711(PCMA/PCMU).
        //3. Secure token for session based authentication, leave empty if none. Should not be used unless Media Server and Web Server run on different domains.
        //4. IP address of Unreal Media Server. If you specify domain name (domain name must be used for signaling over secure websocket) AND your server is behind NAT, then you need to make a change in unrealwebrtcplayer.js, described in http://www.umediaserver.net/phpBB3/viewtopic.php?f=29&t=3563
        //5. Main port of Unreal Media Server for signaling over websocket (by default, 5119 for unsecure webscokets / 443 for secure websockets). Make sure these ports are open in the firewall.
        //6. Use secure websocket for signaling; if true, make sure to set the port to secure port (443 by default) and configure SSL certificate in Unreal Media Server. If this parameter is false and this webpage is loaded under https:// then browsers will display "mixed content" warning; you will need to allow mixed content in your browser.
        //7. Use single central port for actual WebRTC ICE connection. If true, the web browser will receive streaming media from predefined single port in Unreal Media Server, dedicated to WebRTC connections (by default 5135). If true, you only need to open port 5135 in firewall/NAT router for Unreal Media Server (together with signaling ports, 5119 or 443). If false, the WebRTC connection will be made over a random port; you will need to open all ports in the firewall, or add Unreal Media Server as an allowed app in the firewall.
        //8. WebRTC transport protocol. Set to "udp" or "tcp". The central port can be used for both udp and tcp.

        webrtcPlayer = new UnrealWebRTCPlayer("remoteVideo", "Wyse", "", "68.50.44.149", "5119", false, true, "tcp");

        //Comment out next line not to start playing when webpage loads. Then user will need to click on Play button to play; you may want to use a video element with overlayed Play button - check out our SDK for sample webpages.
        webrtcPlayer.Play();    //Start playing automatically when webpage loads. Notice that video element has a "muted" attribute; this is video-only stream anyway. A muted attribute helps to overcome Chrome's autoplay policy, and is not always needed, as described in http://www.umediaserver.net/phpBB3/viewtopic.php?f=29&t=3578

    </script>      

  </div>

    <br><br><br>
    
    <div style="text-align:center">
      <h2><a style="display:inline-block" href="http://%LINK%/Weather" >Main Menu</a></h2>  
    </div>
      
</body>
</html>
)====";
