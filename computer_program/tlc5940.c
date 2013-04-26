#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include "tlc5940.h"

/*
 * Pin Mapping:
 * 
 * 0 -> SIN
 * 4 -> SCLK
 * 2 -> XLAT
 * 3 -> BLANK
 */

#define SIN 0
#define SCLK 4
#define XLAT 2
#define BLANK 3

#define DELAY 0

int tlcleds[NUM_TLCS * 24];
int xlat_needed = 0;
int tlcDone = 0;

void setLed(int ledIndex, int value)
{
	//printf("Led %i set to %i\n", ledIndex, value);
	if(ledIndex >= 0 && ledIndex < NUM_TLCS * 24 && value >= 0 && value < 4096)
		tlcleds[ledIndex] = value;
}

void setAllLeds(int value)
{
	//printf("All LEDs set to %i\n", value);
	if(value >= 0 && value < 4096)
		for(int i = 0; i < NUM_TLCS * 24; i++)
			tlcleds[i] = value;
}

void clearLeds()
{
	//printf("LEDs cleared\n");
	setAllLeds(0);
}

int getLedValue(int index)
{
	if(index >= 0 && index < NUM_TLCS * 24)
		return index;
	return -1;
}

void pulsePin(int pin)
{
	//printf("Pulsing pin number %i\n",pin);
	digitalWrite(pin, 1);
	digitalWrite(pin, 0);
}

void tlc5940init()
{
	//printf("Setup started\n");
	if(wiringPiSetup() == -1)
		printf("Wiring setup failed!\n");
	
	memset(&tlcleds, 0, sizeof(int));

	pinMode(SIN, OUTPUT);
	pinMode(SCLK, OUTPUT);
	pinMode(XLAT, OUTPUT);
	pinMode(BLANK, OUTPUT);
	
	//printf("Setup complete\n");
}

void tlc5940cleanup()
{
	setAllLeds(0);
	updateLeds();

	tlcDone = 1;
}

void updateLeds()
{
	//printf("Updating LEDs\n");
	for(int i = NUM_TLCS * 24 - 1; i >= 0; i--)
	{
		digitalWrite(SIN, tlcleds[i] & 2048);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 1024);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 512);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 256);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 128);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 64);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 32);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 16);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 8);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 4);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 2);
		pulsePin(SCLK);
		digitalWrite(SIN, tlcleds[i] & 1);
		pulsePin(SCLK);
	}

	/*digitalWrite(SIN,1);
	for(int i = NUM_TLCS * 16 - 1; i >= 0; i--)
	{
		pulsePin(SCLK);
	}
	digitalWrite(SIN,0);*/
	//xlat_needed = 1;

	digitalWrite(BLANK, 1);
	pulsePin(XLAT);
	digitalWrite(BLANK, 0);

	//for(int i = 0; i < 4096 * 2; i++)
	//{
	//	pulsePin(GSCLK);
	//	printf("%i\n",i);
	//}
}
/*
int main(void)
{
	printf("Started\n");
	printf("Num Tlcs: %i\n", NUM_TLCS);

	tlc5940init();

	for(int i = 0; i < 4096; i+= 1)
	{
		setAllLeds(i);
		updateLeds();
		usleep(1000);
	}*/

	/*setAllLeds(4095);
	updateLeds();
	usleep(100000000);*/
	//setAllLeds(0);
	//updateLeds();
//}
