#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include "tlc5947.h"
#include <time.h>

/*
 * Basic Use:
 * Call tlc5947init() before trying to control the chip.
 * Use setLed, setAllLeds, and clearLeds to change the outputs.
 * Once you're happy with the values of each output, call updateLeds() to send this information to the chip.
 * Once you're done, call tlc5947cleanup().
 *
 * To use more than one TLC5947, modify NUM_TLCS in the header file and recompile.
 *
 * Pin Mapping:
 * 
 * 0 -> SIN
 * 4 -> SCLK
 * 2 -> XLAT
 * 3 -> BLANK
 *
 * Note: The above follows WiringPi's numbering convention, just google it or something.
 */

#define SIN 0
#define SCLK 4
#define XLAT 2
#define BLANK 3

int tlcleds[NUM_TLCS * 24];
int xlat_needed = 0;
int tlcDone = 0;
struct timespec* sleepTime;

//ledIndex >= 0 && ledIndex < NUM_TLCS * 24
//value >= 0 && value < 4096
void setLed(int ledIndex, int value)
{
	if(ledIndex >= 0 && ledIndex < NUM_TLCS * 24 && value >= 0 && value < 4096)
		tlcleds[ledIndex] = value;
}

//value >= 0 && value < 4096
void setAllLeds(int value)
{
	if(value >= 0 && value < 4096)
		for(int i = 0; i < NUM_TLCS * 24; i++)
			tlcleds[i] = value;
}

void clearLeds()
{
	setAllLeds(0);
}

//index >= 0 && index < NUM_TLCS * 24
int getLedValue(int index)
{
	if(index >= 0 && index < NUM_TLCS * 24)
		return index;
	return -1;
}

void pulsePin(int pin)
{
	digitalWrite(pin, 1);
	//nanosleep(sleepTime, NULL);
	digitalWrite(pin, 0);
}

void tlc5947init()
{
	if(wiringPiSetup() == -1)
		printf("Wiring setup failed!\n");
	
	memset(&tlcleds, 0, sizeof(int));

	pinMode(SIN, OUTPUT);
	pinMode(SCLK, OUTPUT);
	pinMode(XLAT, OUTPUT);
	pinMode(BLANK, OUTPUT);


	sleepTime = malloc(sizeof(struct timespec));
	sleepTime->tv_sec = 0;
	sleepTime->tv_nsec = 5;
}

void tlc5947cleanup()
{
	setAllLeds(0);
	updateLeds();

	tlcDone = 1;
}

void updateLeds()
{
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
	digitalWrite(BLANK, 1);
	pulsePin(XLAT);
	digitalWrite(BLANK, 0);
//	usleep(500000);
}
