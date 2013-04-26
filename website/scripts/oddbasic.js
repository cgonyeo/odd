/*
 * This script works for all basic pages in the web UI to control ODD.
 * It's main limitation is that it will only allow the user to use one
 * animation at a time. 
 *
 * It uses Websockets to connect to a python server which will relay
 * messages to and from the C server that controls ODD. Whenever a 
 * slider is moved, we record the new value and send a message to the
 * server with an update to our one animation.
 *
 * TODO: If we're not connected to the server, provide a button to
 *       reconnect, and/or attempt to reconnect every few seconds.
 */
var ws;
var connected = false;
var animation;
var speed = 50;
var radius = 15;
var R = 10;
var G = 10;
var B = 10;
var modifier;

$(document).ready(function () {
	//Host is currently hardcoded to the PI I'm using
	var host = 'dgonyeoraspi.csh.rit.edu';
	var port = '8888';
	var uri = '/ws';

	ws = new WebSocket("ws://" + host + ":" + port + uri);

	//We will eventually receive updates from the server
	ws.onmessage = function(evt) {
	//	alert("message received: " + evt.data)
	};

	//Alert the user if we disconnect
	ws.onclose = function(evt) { 
		connected = false;
		statii = document.getElementsByClassName("status")
		for(var i = 0; i < statii.length; i++)
			statii[i].innerHTML="Not Connected";
	};

	//Once we connect, tell the user, and tell the server to stop all 
	//current animations and start our animation.
	ws.onopen = function(evt) { 
		connected = true;
		statii = document.getElementsByClassName("status")
		for(var i = 0; i < statii.length; i++)
			statii[i].innerHTML="Connected";
	
		/*
		var temp = speed;
		if(temp > 50)
			temp += (temp - 50) * 3;
		temp /= 50;
		var temp2 = radius;
		temp2 /= 10;
		ws.send("stop !" + animation + " " + temp + " " + temp2 + " " + R + " " + G + " " + B + " " + modifier + " !");
		*/
	};

});

$(document).delegate('#setall', 'pageshow', function() {
	animation = 'setall';
	speed = 50;
	radius = 15;
	R = 10;
	G = 10;
	B = 10;
	modifier = 'add';

	var temp = speed;
	if(temp > 50)
		temp += (temp - 50) * 3;
	temp /= 50;
	var temp2 = radius;
	temp2 /= 10;
	ws.send("stop !" + animation + " " + temp + " " + temp2 + " " + (R*16) + " " + (G*16) + " " + (B*16) + " " + modifier + " !");
});

$(document).delegate('#pulse', 'pageshow', function() {
	animation = 'smoothstrobe';
	speed = 50;
	radius = 15;
	R = 10;
	G = 10;
	B = 10;
	modifier = 'add';

	var temp = speed;
	if(temp > 50)
		temp += (temp - 50) * 3;
	temp /= 50;
	var temp2 = radius;
	temp2 /= 10;
	ws.send("stop !" + animation + " " + temp + " " + temp2 + " " + (R*16) + " " + (G*16) + " " + (B*16) + " " + modifier + " !");
});

$(document).delegate('#strobe', 'pageshow', function() {
	animation = 'strobe';
	speed = 50;
	radius = 15;
	R = 10;
	G = 10;
	B = 10;
	modifier = 'add';

	var temp = speed;
	if(temp > 50)
		temp += (temp - 50) * 3;
	temp /= 50;
	var temp2 = radius;
	temp2 /= 10;
	ws.send("stop !" + animation + " " + temp + " " + temp2 + " " + (R*16) + " " + (G*16) + " " + (B*16) + " " + modifier + " !");
});

$(document).delegate('#cyloneye', 'pageshow', function() {
	animation = 'cyloneye';
	speed = 50;
	radius = 15;
	R = 10;
	G = 10;
	B = 10;
	modifier = 'add';

	var temp = speed;
	if(temp > 50)
		temp += (temp - 50) * 3;
	temp /= 50;
	var temp2 = radius;
	temp2 /= 10;
	ws.send("stop !" + animation + " " + temp + " " + temp2 + " " + (R*16) + " " + (G*16) + " " + (B*16) + " " + modifier + " !");
});

//Sends an updated animation to the server
function send()
{
	var temp = speed;
//	if(temp > 50)
//		temp += (temp - 50) * 3;
	temp /= 50;
	var temp2 = radius;
	temp2 /= 10;
	if(connected)
		ws.send("update " + temp + " " + temp2 + " " + (R*16) + " " + (G*16) + " " + (B*16) + " 0 !");
}

//Radius slider has changed.
function RAupdate(newValue)
{
	radius = newValue;
	send();
}
//Speed slider has changed.
function Supdate(newValue)
{
	speed = newValue;
	send();
}
//Red slider has changed.
function Rupdate(newValue)
{
	R = newValue;
	send();
}
//Green slider has changed.
function Gupdate(newValue)
{
	G = newValue;
	send();
}
//Blue slider has changed.
function Bupdate(newValue)
{
	B = newValue;
	send();
}
