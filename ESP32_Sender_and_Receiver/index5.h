//index5.h
const char HTML5[] PROGMEM = R"====(
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd"> <html lang="en"> <head> <meta http-equiv="content-type" content="text/html; charset=utf-8"> <title>Contact us</title> </head> <body> 

<form method="post" action="//submit.form" onSubmit="return validateForm();">
<div style="width: 400px;">
</div>
<div style="padding-bottom: 18px;font-size : 24px;">Contact Us</div>
<div style="display: flex; padding-bottom: 18px;width : 450px;">
<div style=" margin-left : 0; margin-right : 1%; width : 49%;">First name<span style="color: red;"> *</span><br/>
<input type="text" id="data_12" name="data_12" style="width: 100%;" class="form-control"/>
</div>
<div style=" margin-left : 1%; margin-right : 0; width : 49%;">Last name<span style="color: red;"> *</span><br/>
<input type="text" id="data_13" name="data_13" style="width: 100%;" class="form-control"/>
</div>
</div><div style="padding-bottom: 18px;">Email<span style="color: red;"> *</span><br/>
<input type="text" id="data_4" name="data_4" style="width : 550px;" class="form-control"/>
</div>
<div style="padding-bottom: 18px;">Comments and Questions<br/>
<textarea id="data_11" false name="data_11" style="width : 550px;" rows="6" class="form-control"></textarea>
</div>
<div style="padding-bottom: 18px;"><input name="skip_Submit" value="Submit" type="submit"/></div>
<div>
<div style="float:right"><a href="https://www.100forms.com" id="lnk100" title="form to email">form to email</a></div>
<script src="https://www.100forms.com/js/FORMKEY:RUQ7YJN36S5Z" type="text/javascript"></script>
</div>
</form>

<script type="text/javascript">
function validateForm() {
if (isEmpty(document.getElementById('data_12').value.trim())) {
alert('First name is required!');
return false;
}
if (isEmpty(document.getElementById('data_13').value.trim())) {
alert('Last name is required!');
return false;
}
if (isEmpty(document.getElementById('data_4').value.trim())) {
alert('Email is required!');
return false;
}
if (!validateEmail(document.getElementById('data_4').value.trim())) {
alert('Email must be a valid email address!');
return false;
}
return true;
}
function isEmpty(str) { return (str.length === 0 || !str.trim()); }
function validateEmail(email) {
var re = /^([\w-]+(?:\.[\w-]+)*)@((?:[\w-]+\.)*\w[\w-]{0,66})\.([a-z]{2,15}(?:\.[a-z]{2})?)$/i;
return isEmpty(email) || re.test(email);
}
</script>

<br><br>
<a href=http://%LINK%/Weather >Home</a>
<br>

</body> </html>

)====";
