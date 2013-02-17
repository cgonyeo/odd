#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define NUM_LEDS 16

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
	//	system("clear");
		printf("%d, ", leds[i]);
		fflush(NULL);
	}
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
void cylonEye(double speed, double radius, double totalTime) {
	totalTime *= speed;
	double eyeLoc = 0.0;
	for(int i = 0; i < NUM_LEDS; i++)
		tempLeds[i] = 0;
	if(((int)totalTime / NUM_LEDS) % 2 == 1)
		eyeLoc = remainder(totalTime, NUM_LEDS);
	else
		eyeLoc = NUM_LEDS - 1 - remainder(totalTime, NUM_LEDS);
	
	double ledDistances[NUM_LEDS];
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(eyeLoc - i > 0)
			ledDistances[i] = eyeLoc - i;
		else
			ledDistances[i] = i - eyeLoc;
		ledDistances[i] -= NUM_LEDS;
		ledDistances[i] *= -1;
		ledDistances[i] -= NUM_LEDS - radius;
		ledDistances[i] *= 1 / radius;
		ledDistances[i] *= 254;
	}
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(ledDistances[i] > 0)
			tempLeds[i] = (char)ledDistances[i];
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
		cylonEye(10, 2, totalTime);
		addLeds();
		if(failed==0)
			write_odd(fp, leds);
		else
			write_console(leds);
	}
	
	//and we're done, close the stream.
	if(failed==0)
		fclose(fp);
}

