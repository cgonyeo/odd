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

var animationsAvailable = ["setall","smoothstrobe","strobe","cyloneye"];
var modifiersAvailable = ["add","subtract"];

$(document).ready(function () {
	$('#addButton').click(function() {
		displayNewAnimation("setall", 1, 1, 0, 0, 0, "add");
	});
});
	//Host is currently hardcoded to the PI I'm using
/*	var host = 'dgonyeoraspi.csh.rit.edu';
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
	/*};

});
*/

function isNumber(possiblyANum)
{
	return ! isNaN (possiblyANum-0) && possiblyANum !== null && possiblyANum !== "" && possiblyANum !== false;
}

function displayNewAnimation(animationName, speed, radius, red, green, blue, animationModifier)
{
	if(isNumber(speed) && isNumber(radius) && isNumber(red) && isNumber(green) && isNumber(blue) && $.inArray(animationName, animationsAvailable) > -1 && $.inArray(animationModifier, modifiersAvailable) > -1)
	{
		
		 var temp = $('#firstAnimation').clone();
		 $(temp).removeAttr("id");
		 $(temp).appendTo('#animationList');

		/*var animationList = $('#animationList').children;
		var animationParts = animationList[0].children;
		animationParts[animationParts.length - 1].click(function() {
			$(this).parent.remove();
		});*/
		var animations = document.querySelectorAll('.animation');
		var lastAnim = animations[animations.length - 1];
		var X = lastAnim.querySelectorAll('.ximg')[0];
		console.log($(X).parent());

		$(X).click(function() {
			$(this).parent().remove();
		});
	}
}

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
		ws.send("update " + temp + " " + temp2 + " " + R + " " + G + " " + B + " 0 !");
}
