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
	
	$('#applyButton').click(function() {
		sendUpdate();
	});
	
	//Host is currently hardcoded to the PI I'm using
	var host = 'dgonyeoraspi.csh.rit.edu';
	var port = '8888';
	var uri = '/ws';

	ws = new WebSocket("ws://" + host + ":" + port + uri);

	//We will eventually receive updates from the server
	ws.onmessage = function(evt) {
		alert(evt.data);
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
		
		ws.send("stop !");
	};
});

function sendUpdate()
{
	ws.send("stop !");

	var animations = document.querySelectorAll('.animation');
	for(var i = 1; i < animations.length; i++)
	{
		animSettings = $(animations[i]).children();
		var nameIndex = $(animSettings[0]).children()[1].selectedIndex;
		var animName = "no animation";
		switch(nameIndex)
		{
			case 0:
				animName = "setall";
				break;
			case 1:
				animName = "strobe";
				break;
			case 2:
				animName = "smoothstrobe";
				break;
			case 3:
				animName = "cyloneye";
				break;
		}
		
		var speedVal = $(animSettings[1]).children()[1].value;
		var radiusVal = $(animSettings[2]).children()[1].value;
		var redVal = $(animSettings[3]).children()[1].value;
		var greenVal = $(animSettings[4]).children()[1].value;
		var blueVal = $(animSettings[5]).children()[1].value;

		var modifierIndex = $(animSettings[6]).children()[1].selectedIndex;
		var modifierName = "no modifier";
		switch(modifierIndex)
		{
			case 0:
				modifierName = "add";
				break;
			case 1:
				modifierName = "subtract";
				break;
		}

		var animationString = animName + " " + speedVal + " " + radiusVal + " " + redVal + " " + greenVal + " " + blueVal + " " + modifierName + " !";
		ws.send(animationString);
	}
}

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

		var animations = document.querySelectorAll('.animation');
		var lastAnim = animations[animations.length - 1];
		var X = lastAnim.querySelectorAll('.ximg')[0];

		$(X).click(function() {
			$(this).parent().remove();
		});
	}
}
