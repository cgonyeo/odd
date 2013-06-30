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
#include <math.h>

//Helper method. Returns the length of the array each animation is expecting in double* params.
//If the function is not found, returns -1
int numParams(void (*function)(double*, double, odd_led_t*, odd_led_t *[NUM_LEDS]))
{

	if(function == cylonEye)
		return 2;
	if(function == cylonEye_Linear)
		return 2;
	if(function == strobe)
		return 1;
	if(function == setAll)
		return 0;
	if(function == smoothStrobe)
		return 1;
	if(function == sinAnimation)
		return 1;
	if(function == volumeAnimation)
		return 0;
	if(function == dammitAnimation)
		return 0;
	return -1;
}

//Animation: Lights up a clump of LEDs and then travels back and forth across the row
//Follows a basic sin curve (moves faster in the middle, slower at the ends)
void cylonEye(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]) {
	double speed = params[0];
	double radius = params[1];

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
	center = odd_sin(center);
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
void cylonEye_Linear(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS]) {
	double speed = params[0];
	double radius = params[1];
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
void strobe(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	double speed = params[0];
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
void setAll(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	(void)params;
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = color->R;
		tempLeds[i]->G = color->G;
		tempLeds[i]->B = color->B;
	}
}

//Animation: Like strobe but fades LEDs in and out and pauses when they're off.
void smoothStrobe(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	double speed = params[0];

	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = odd_remainder(time, 1);
	else
		power = 1 - odd_remainder(time, 1);
	power = odd_pow(power, 2);
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
void sinAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	double speed = params[0];

	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = odd_remainder(time, 1);
	else
		power = 1 - odd_remainder(time, 1);
	power = odd_sin(power);
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = color->R * power;
		tempLeds[i]->G = color->G * power;
		tempLeds[i]->B = color->B * power;
	}
}

void volumeAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	(void)params;
	(void)totalTime;
	
	double avg = 0;

	SAMPLE soundBuffer[FFT_INPUT_SIZE];
	getSoundBuffer(soundBuffer);

	for(int i = 0; i < FFT_INPUT_SIZE; i++)
	{
		if(soundBuffer[i] > 0)
			avg += soundBuffer[i];
		else
			avg -= soundBuffer[i];
	}
	avg /= FFT_INPUT_SIZE;
	avg = pow(avg, 2);
	avg *= 4096;

	double r = color->R / 4095.0;
	double g = color->G / 4095.0;
	double b = color->B / 4095.0;

	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = r * avg;
		tempLeds[i]->G = g * avg;
		tempLeds[i]->B = b * avg;
	}
}

void dammitAnimation(double* params, double totalTime, odd_led_t* color, odd_led_t* tempLeds[NUM_LEDS])
{
	(void)params;

	SAMPLE soundBuffer[FFT_OUTPUT_SIZE];
	runFFT(soundBuffer);
	int binsPerLed = 20;
	float ledsPerSection = NUM_LEDS / 9.0f;
	
	for(int i = 0; i < 9; i++)
	{
		int lowerBin = 0;
		int upperBin = 10;
		switch(i)
		{
			case 0:
				lowerBin = 0;
				upperBin = 11;
				break;
			case 1:
				lowerBin = 11;
				upperBin = 22;
				break;
			case 2:
				lowerBin = 22;
				upperBin = 42;
				break;
			case 3:
				lowerBin = 42;
				upperBin = 74;
				break;
			case 4:
				lowerBin = 74;
				upperBin = 139;
				break;
			case 5:
				lowerBin = 139;
				upperBin = 416;
				break;
			case 6:
				lowerBin = 416;
				upperBin = 833;
				break;
			case 7:
				lowerBin = 833;
				upperBin = 1204;
				break;
			case 8:
				lowerBin = 1204;
				upperBin = 1481;
				break;
		}


		double avg = 0;
		for(int j = lowerBin; j < upperBin; j++)
		{
		//	//printf("%i\n", j);
			avg += soundBuffer[j] * 10;
		}
		avg /= upperBin - lowerBin;
		avg *= pow((i + 1.0) / 2.0, 2);
		avg = pow(avg / 4096.0, 2) * 4096;
		tempLeds[2 * i + 1]->G = avg;
		tempLeds[2 * i + 2]->G = avg;

		double r = 0;
		double g = 0;
		double b = 0;
		r = odd_remainder(totalTime, 3.0);
		g = odd_remainder(totalTime + 1.0, 3.0);
		b = odd_remainder(totalTime + 2.0, 3.0);

		//r = r + 0.0;
		//if(r < 0)
		//	r *= -1;
		printf("%f\n", r);
	}



	//float r = 0;
	//float g = 0;
	//float b = 0;
	//float index = odd_remainder(totalTime, 6);
	//r = abs(1.0-index) * -1 + 1;
	//g = abs(3.0-index) * -1 + 1;
	//b = abs(5.0-index) * -1 + 1;
	//if(r < 0)
	//	r = 0;
	//if(g < 0)
	//	g = 0;
	//if(b < 0)
	//	b = 0;
	//r *= 4096;
	//g *= 4096;
	//b *= 4096;

	//printf("%f, %f, %f\n", r, g, b);

	//for(int i = 0; i < NUM_LEDS; i++) 
	//{
	//	float avg = 0;
	//	for(int j = i * binsPerLed; j < (i + 1) * binsPerLed; j++)
	//	{
	//		int temp = soundBuffer[j] * 100;
	//		if(temp < 0)
	//			temp *= -1;
	//		avg += temp;
	//	}
	//	avg /= binsPerLed;

	//	avg = odd_pow(avg / 4096.0, 2) * 4096;

	//	tempLeds[i]->R = avg;
	//}
}
