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
		if(evt.data != "Not much, you?")
		{
			var tokens = evt.data.split(/ |!/);
			console.log(tokens);
			for(var i = 0; i < tokens.length; i+= 8)
			{
				displayNewAnimation(tokens[i], tokens[i+1], tokens[i+2], (tokens[i+3]/16), (tokens[i+4]/16), (tokens[i+5]/16), tokens[i+6]);
			}
		}
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
		
		ws.send("sup !");
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

		if(isNumber(speedVal) && isNumber(radiusVal) && isNumber(redVal) && isNumber(greenVal) && isNumber(blueVal))
		{
			var animationString = animName + " " + speedVal + " " + radiusVal + " " + (redVal*16) + " " + (greenVal*16) + " " + (blueVal*16) + " " + modifierName + " !";
			ws.send(animationString);
		}
	}
}

function isNumber(possiblyANum)
{
	return ! isNaN (possiblyANum-0) && possiblyANum !== null && possiblyANum !== "" && possiblyANum !== false;
}

function displayNewAnimation(animationName, newSpeed, newRadius, red, green, blue, animationModifier)
{
	if(isNumber(newSpeed) && isNumber(newRadius) && isNumber(red) && isNumber(green) && isNumber(blue) && $.inArray(animationName, animationsAvailable) > -1 && $.inArray(animationModifier, modifiersAvailable) > -1)
	{
		var temp = $('#firstAnimation').clone();
		$(temp).removeAttr("id");
		$(temp).appendTo('#animationList');

		var animations = document.querySelectorAll('.animation');
		var lastAnim = animations[animations.length - 1];

		var animSelector = lastAnim.querySelectorAll('.animNameSelector')[0];
		switch(animationName)
		{
			case "setall":
				animSelector.selectedIndex = 0;
				break;
			case "strobe":
				animSelector.selectedIndex = 1;
				animName = "strobe";
				break;
			case "strobestrobe":
				animSelector.selectedIndex = 2;
				animName = "smoothstrobe";
				break;
			case "cyloneye":
				animSelector.selectedIndex = 3;
				animName = "cyloneye";
				break;
		}
		console.log("Anim selector: " + animSelector);

		var newSpeedInput = lastAnim.querySelectorAll('.speed')[0];
		newSpeedInput.value = newSpeed;
		console.log("Speed input: " + newSpeedInput);
		var newRadiusInput = lastAnim.querySelectorAll('.radius')[0];
		newRadiusInput.value = newRadius;
		console.log("Radius input: " + newRadiusInput);
		var redInput = lastAnim.querySelectorAll('.red')[0];
		redInput.value = red;
		console.log("Red input: " + redInput);
		var greenInput = lastAnim.querySelectorAll('.green')[0];
		greenInput.value = green;
		console.log("Green input: " + greenInput);
		var blueInput = lastAnim.querySelectorAll('.blue')[0];
		blueInput.value = blue;
		console.log("blue input: " + blueInput);

		var animModSelector = lastAnim.querySelectorAll('.animModSelector')[0];
		switch(animationName)
		{
			case "add":
				animModSelector.selectedIndex = 0;
				break;
			case "subtract":
				animModSelector.selectedIndex = 1;
				animName = "strobe";
				break;
		}
		console.log("AnimMod selector: " + animModSelector);

		var X = lastAnim.querySelectorAll('.ximg')[0];
		$(X).click(function() {
			$(this).parent().remove();
		});
	}
}
