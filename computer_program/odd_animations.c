#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "portaudio.h"
#include "odd_audio.h"
#include "odd_data_types.h"
#include "odd_math.h"
#include "odd_animations.h"

//Animation: Lights up a clump of LEDs and then travels back and forth across the row
//Follows a basic sin curve (moves faster in the middle, slower at the ends)
void cylonEye(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]) {
	//scale the time by our speed to alter the rate of tf the animation
	double time = totalTime * speed;
	//double to keep track of the location of the center
	double center = 0.0;
	//we want the center to go between 0 and NUM_LEDS - 1, due to 0 based indexing of leds.
	int numLeds = NUM_LEDS - 1;
	//clear the tempLeds array so we can use it
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = 0;
		tempLeds[i]->G = 0;
		tempLeds[i]->B = 0;
	}
	
	//Calculate the center
	if((int)time % 2 == 1)
		center = odd_remainder(time, 1);
	else
		center = 1 - odd_remainder(time, 1);
	center = sin(center);
	center *= numLeds;
	
	
	//Calculate the distance of each LED from the center, and do some math to figure out each LED's brightness
	double ledDistances[NUM_LEDS];
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(center - i > 0)
			ledDistances[i] = center - i;
		else
			ledDistances[i] = i - center;
		ledDistances[i] -= numLeds;
		ledDistances[i] *= -1;
		ledDistances[i] -= numLeds - radius;
		ledDistances[i] *= 1 / radius;
	}
	//If an LED has a positive brightness, set it.
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(ledDistances[i] > 0)
		{
			tempLeds[i]->R = color->R * ledDistances[i];
			tempLeds[i]->G = color->G * ledDistances[i];
			tempLeds[i]->B = color->B * ledDistances[i];
		}
	}
}

//Animation: same as cylonEye but follows a linear movement (constant speed)
void cylonEye_Linear(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]) {
	//scale the time by our speed to alter the rate of tf the animation
	double time = totalTime * speed;
	//double to keep track of the location of the center
	double center = 0.0;
	//we want the center to go between 0 and NUM_LEDS - 1, due to 0 based indexing of leds.
	int numLeds = NUM_LEDS - 1;
	//clear the tempLeds array so we can use it
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = 0;
		tempLeds[i]->G = 0;
		tempLeds[i]->B = 0;
	}
	
	//Calculate the center
	if((int)time % 2 == 1)
		center = odd_remainder(time, 1);
	else
		center = 1 - odd_remainder(time, 1);
	center *= numLeds;
	
	
	//Calculate the distance of each LED from the center, and do some math to figure out each LED's brightness
	double ledDistances[NUM_LEDS];
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(center - i > 0)
			ledDistances[i] = center - i;
		else
			ledDistances[i] = i - center;
		ledDistances[i] -= numLeds;
		ledDistances[i] *= -1;
		ledDistances[i] -= numLeds - radius;
		ledDistances[i] *= 1 / radius;
	}
	//If an LED has a positive brightness, set it.
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(ledDistances[i] > 0)
		{
			tempLeds[i]->R = color->R * ledDistances[i];
			tempLeds[i]->G = color->G * ledDistances[i];
			tempLeds[i]->B = color->B * ledDistances[i];
		}
	}
}

//Animation: Turns the LEDs off and on and off and on and off and on and off...
void strobe(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	(void)radius;
	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = 1;
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = color->R * power;
		tempLeds[i]->G = color->G * power;
		tempLeds[i]->B = color->B * power;
	}
}

//Animation: Sets all LEDs to color
void setAll(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	(void)radius;
	(void)speed;
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = color->R;
		tempLeds[i]->G = color->G;
		tempLeds[i]->B = color->B;
	}
}

//Animation: Like strobe but fades LEDs in and out and pauses when they're off.
void smoothStrobe(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	(void)radius;
	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = odd_remainder(time, 1);
	else
		power = 1 - odd_remainder(time, 1);
	power = pow(power, 2);
	power *= 1.5;
	power -= 0.5;
	if(power < 0)
		power = 0;
	for(int i = 0; i < NUM_LEDS; i++) {
		tempLeds[i]->R = color->R * power;
		tempLeds[i]->G = color->G * power;
		tempLeds[i]->B = color->B * power;
	}
}

//Animation that makes the brightness of the LEDs follow a sin curve
void sinAnimation(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	(void)radius;
	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = odd_remainder(time, 1);
	else
		power = 1 - odd_remainder(time, 1);
	power = sin(power);
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = color->R * power;
		tempLeds[i]->G = color->G * power;
		tempLeds[i]->B = color->B * power;
	}
}

void dammitAnimation(double speed, double radius, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	(void)speed;
	(void)radius;
	(void)totalTime;
	
	double avg = 0;

	SAMPLE soundBuffer[FRAMES_PER_BUFFER];
	getSoundBuffer(soundBuffer);

	for(int i = 0; i < FRAMES_PER_BUFFER; i++)
	{
		if(soundBuffer[i] > 0)
			avg += soundBuffer[i];
		else
			avg -= soundBuffer[i];
	}
	avg /= FRAMES_PER_BUFFER;
	avg *= 2;
	//printf("Average: %f\n", avg);

	double r = color->R / 4095;
	double g = color->G / 4095;
	double b = color->B / 4095;

	double scale = 1;

	int biggest = 0;
	if(g > r && g > b)
		biggest += 1;
	if(b > r && b > g)
		biggest += 2;

	switch(biggest)
	{
		case 0:
			scale = 100 / r;
			break;
		case 1:
			scale = 100 / g;
			break;
		case 2:
			scale = 100 / b;
			break;
		default:
			scale = 100;
	}

	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = color->R * avg;
		tempLeds[i]->G = color->G * avg;
		tempLeds[i]->B = color->B * avg;
	}
}
