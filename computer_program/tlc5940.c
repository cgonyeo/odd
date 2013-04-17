#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include "tlc5940.h"

/*
 * Pin Mapping:
 * 
 * 0 -> SIN
 * 4 -> SCLK
 * 2 -> XLAT
 * 3 -> BLANK
 * 1 -> GSCLK
 */

#define SIN 0
#define SCLK 4
#define XLAT 2
#define BLANK 3
#define GSCLK 1
#define PWM_ON 512
#define PWM_OFF 0

#define DELAY 0

int tlcleds[NUM_TLCS * 16];
int xlat_needed = 0;
int tlcDone = 0;
pthread_t ul;

void setLed(int ledIndex, int value)
{
	//printf("Led %i set to %i\n", ledIndex, value);
	if(ledIndex >= 0 && ledIndex < NUM_TLCS * 16 && value >= 0 && value < 4096)
		tlcleds[ledIndex] = value;
}

void setAllLeds(int value)
{
	//printf("All LEDs set to %i\n", value);
	if(value >= 0 && value < 4096)
		for(int i = 0; i < NUM_TLCS * 16; i++)
			tlcleds[i] = value;
}

void clearLeds()
{
	//printf("LEDs cleared\n");
	setAllLeds(0);
}

int getLedValue(int index)
{
	if(index >= 0 && index < NUM_TLCS * 16)
		return index;
	return -1;
}

void pulsePin(int pin)
{
	//printf("Pulsing pin number %i\n",pin);
	digitalWrite(pin, 1);
	digitalWrite(pin, 0);
}

void *tlcUpdateLoop(void *arg)
{
	while(!tlcDone)
	{
		/*for(int i = 0; i < 4096 * 2; i++)
		{
			pulsePin(GSCLK);
		}
		pulsePin(BLANK);
		if(xlat_needed)
		{
			digitalWrite(BLANK, 1);
			pulsePin(XLAT);
			digitalWrite(BLANK, 0);
			xlat_needed = 0;
		}*/
		usleep(2000);
		pwmWrite(GSCLK, PWM_OFF);
		pulsePin(BLANK);
		pwmWrite(GSCLK, PWM_ON);
	}
	return NULL;
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
	pinMode(GSCLK, PWM_OUTPUT);
	
	pwmWrite(GSCLK, PWM_ON);
	pthread_create(&ul,NULL,tlcUpdateLoop,"randomargs");
	
	//printf("Setup complete\n");
}

void tlc5940cleanup()
{
	setAllLeds(0);
	updateLeds();
	digitalWrite(GSCLK, PWM_OFF);

	tlcDone = 1;
	pthread_join(ul, NULL);
}

void updateLeds()
{
	//printf("Updating LEDs\n");
	for(int i = NUM_TLCS * 16 - 1; i >= 0; i--)
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

	pwmWrite(GSCLK, PWM_OFF);
	digitalWrite(BLANK, 1);
	pulsePin(XLAT);
	digitalWrite(BLANK, 0);
	pwmWrite(GSCLK, PWM_ON);

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
