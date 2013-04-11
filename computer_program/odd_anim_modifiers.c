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
#include "odd_data_types.h"
#include "odd_math.h"
#include "odd_animations.h"
#include "odd_anim_modifiers.h"

//Inverts all channels on the temp LEDS.
//For example:
//4095 -> 0
//0 -> 4095
void invertTempLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS])
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i]->R = 4095 - tempLeds[i]->R;
		tempLeds[i]->G = 4095 - tempLeds[i]->G;
		tempLeds[i]->B = 4095 - tempLeds[i]->B;
	}
}

//Adds the current modifications to the LEDs to the led array
void addLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS])
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(leds[i]->R + tempLeds[i]->R > 4095)
			leds[i]->R = 4095;
		else
			leds[i]->R += tempLeds[i]->R;
		
		if(leds[i]->G + tempLeds[i]->G > 4095)
			leds[i]->G = 4095;
		else
			leds[i]->G += tempLeds[i]->G;
		
		if(leds[i]->B + tempLeds[i]->B > 4095)
			leds[i]->B = 4095;
		else
			leds[i]->B += tempLeds[i]->B;
	}
}

//Subtracts the current modifications to the LEDs from the led array
void subtractLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS])
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(leds[i]->R - tempLeds[i]->R < 0)
			leds[i]->R = 0;
		else
			leds[i]->R -= tempLeds[i]->R;
		
		if(leds[i]->G - tempLeds[i]->G < 0)
			leds[i]->G = 0;
		else
			leds[i]->G -= tempLeds[i]->G;
		
		if(leds[i]->B - tempLeds[i]->B < 0)
			leds[i]->B = 0;
		else
			leds[i]->B -= tempLeds[i]->B;
	}
}

//Subtracts the inverse of tempLeds from leds
void inverseSubtractLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS])
{
	invertTempLeds(leds, tempLeds);
	subtractLeds(leds, tempLeds);
	invertTempLeds(leds, tempLeds);
}

//Multiplies tempLeds to leds
void multiplyLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS])
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(leds[i]->R * tempLeds[i]->R > 4095)
			leds[i]->R = 4095;
		else
			leds[i]->R *= tempLeds[i]->R;
		
		if(leds[i]->G * tempLeds[i]->G > 4095)
			leds[i]->G = 4095;
		else
			leds[i]->G *= tempLeds[i]->G;
		
		if(leds[i]->B * tempLeds[i]->B > 4095)
			leds[i]->B = 4095;
		else
			leds[i]->B *= tempLeds[i]->B;
	}
}
