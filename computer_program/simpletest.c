#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_LEDS 32

//typedef struct odd_led_t {
//	unsigned char R;
//	unsigned char G;
//	unsigned char B;
//};

unsigned char leds[NUM_LEDS];
unsigned char tempLeds[NUM_LEDS];

//Writes the led array *leds to the file stream fp.
void write_odd(FILE* fp, unsigned char *leds) {
	for(int i=0; i<NUM_LEDS; i++) {
		fwrite(&leds[i], 1, 1, fp);
		fflush(fp);
	}
	unsigned char end = 255;
	fwrite(&end, 1, 1, fp);
}

//Writes the led array to the console
void write_console(unsigned char *leds) {
	printf("\n");
	for(int i=0; i<NUM_LEDS; i++) {
		printf("%d, ", leds[i]);
		fflush(NULL);
	}
//	system("clear");
}

double remainder(double dividend, int divisor) {
	int quotient = (int) dividend / divisor;
	return dividend - divisor * quotient;
}

double formatTime(long int seconds, long int useconds)
{
	return seconds % 10000 + (useconds - useconds % 1000) / 1000000.0;
}

void addLeds()
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(leds[i] + tempLeds[i] > 254)
			leds[i] = 254;
		else
			leds[i] += tempLeds[i];
	}
}

void resetLeds()
{
	for(int i = 0; i < NUM_LEDS; i++)
		leds[i] = 0;
}

//Simple animation
void cylonEye(double speed, double radius, double strength, double totalTime) {
	//scale the time by our speed to alter the rate of tf the animation
	double time = totalTime * speed;
	//double to keep track of the location of the center
	double center = 0.0;
	//we want the center to go between 0 and NUM_LEDS - 1, due to 0 based indexing of leds.
	int numLeds = NUM_LEDS - 1;
	//clear the tempLeds array so we can use it
	for(int i = 0; i < NUM_LEDS; i++)
		tempLeds[i] = 0;
	
	//Calculate the center
	if(((int)time / numLeds) % 2 == 1)
		center = remainder(time, numLeds);
	else
		center = numLeds - remainder(time, numLeds);
	
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
		ledDistances[i] *= 254 * strength;
		if(ledDistances[i] > 254)
			ledDistances[i] = 254;
		sleep(0.1);
	}
	//If an LED has a positive brightness, set it.
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(ledDistances[i] > 0)
			tempLeds[i] = (unsigned char)ledDistances[i];
	}
}

void strobe(double speed, double strength, double totalTime)
{
	double time = totalTime * speed;
	double power = 0;
	if((int)time % 2 == 1)
		power = 254 * strength;
	for(int i = 0; i < NUM_LEDS; i++)
	{
		tempLeds[i] = (unsigned char)power;
	}
}

int main ( void )
{
	//Open the stream, record if it fails
	int failed = 0;
	FILE *fp = fopen( "/dev/ttyUSB0" , "w" );
	if( fp == NULL )
	{
		failed = 1;
		printf("Failed!");
	}
	
	//Some setup...
	double elapsedTime, totalTime = 0;
	struct timeval start, current, previous;
	gettimeofday(&start, NULL);
	gettimeofday(&previous, NULL);
	for(int i = 0; i < NUM_LEDS; i++)
		leds[i] = 0;
	
	//Run this block in a while loop eventually
	while(1)
	{
		resetLeds();
		previous = current;
		gettimeofday(&current, NULL);
		elapsedTime =  formatTime(current.tv_sec, current.tv_usec) - formatTime(previous.tv_sec, previous.tv_usec);
		totalTime = formatTime(current.tv_sec, current.tv_usec) - formatTime(start.tv_sec, start.tv_usec);
		cylonEye(30, 2, 0.5, totalTime);
		addLeds();
		cylonEye(13, 5, 0.5, totalTime);
		addLeds();
//		strobe(40, 0.5, totalTime);
//		addLeds();
		if(failed==0)
			write_odd(fp, leds);
		else
			write_console(leds);
	}
	
	//and we're done, close the stream.
	if(failed==0)
		fclose(fp);
}

