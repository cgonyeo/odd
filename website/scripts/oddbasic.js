var ws;
var connected = false;
var speed = 50;
var radius = 15;
var R = 10;
var G = 10;
var B = 10;

$(document).ready(function () {

	removeUnusedControls();
	
	//$("#open").click(function(evt) {
	//evt.preventDefault();

	var host = 'localhost';//$("#host").val();
	var port = '8888';//$("#port").val();
	var uri = '/ws';//$("#uri").val();

	ws = new WebSocket("ws://" + host + ":" + port + uri);

	ws.onmessage = function(evt) {
	//	alert("message received: " + evt.data)
	};

	ws.onclose = function(evt) { 
		connected = false;
		document.getElementById("status").innerHTML="Not Connected";
	};

	ws.onopen = function(evt) { 
		connected = true;
		document.getElementById("status").innerHTML="Connected";
		
		var temp = speed;
		if(temp > 50)
			temp += (temp - 50) * 3;
		temp /= 50;
		var temp2 = radius;
		temp2 /= 10;
		ws.send("stop !" + animation + " " + temp + " " + temp2 + " " + R + " " + G + " " + B + " " + modifier + " !");
	};

	/*$("#send").click(function(evt) {
		ws.send($("#input").val());
	});*/

	//});

});

function removeUnusedControls()
{
	if(speedOn == 'n')
		$('#speed').remove()
	if(radiusOn == 'n')
		$('#radius').remove()
}

function send()
{
	var temp = speed;
//	if(temp > 50)
//		temp += (temp - 50) * 3;
	temp /= 50;
	var temp2 = radius;
	temp2 /= 10;
	if(connected)
		ws.send("update " + temp + " " + temp2 + " " + R + " " + G + " " + B + " 0 !");
}

function RAupdate(newValue)
{
	radius = newValue;
	send();
}
function Supdate(newValue)
{
	speed = newValue;
	send();
}
function Rupdate(newValue)
{
	R = newValue;
	send();
}
function Gupdate(newValue)
{
	G = newValue;
	send();
}
function Bupdate(newValue)
{
	B = newValue;
	send();
}
