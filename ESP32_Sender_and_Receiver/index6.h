//index6.h
const char HTML6[] PROGMEM = R"====(
<!DOCTYPE HTML>
<html>

<head>
    <title>Show</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
.main
{
  type: text/txt;
  font-size: 1vw;
  width: auto;
  margin: 30px;  
  height: auto;
}
</style>
</head>

<body>

    <h2>File Reader
    <!-- <a href="/Show"><button>Show Log File</button></a> -->
    <br>
    <br>
    <div class="main" style="white-space: pre-wrap;" type="text/txt" id=file-content></div>
    <!-- <p style="white-space: pre-wrap;"id="file-content"></p>  -->
    <br>  
    <a href=http://%LINK%/SdBrowse >File Reader</a>
    <br>
    <a href=http://%LINK%/Weather >Main Menu</a></h2>   
    
</body>
<script>
  window.addEventListener('load', getFile);
  function getFile(){
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() 
    {
      if (this.readyState == 4 && this.status == 200) 
      {
        console.log(this.responseText);
        document.getElementById("file-content").innerHTML=this.responseText;
      }
    };
  xhr.open("GET", "/get-file", true);
  xhr.send();
}
</script>
</html>
)====";
