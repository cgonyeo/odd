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
 * 0(17) -> SIN
 * 4(23) -> SCLK
 * 2(27) -> XLAT
 * 3(22) -> BLANK
 *
 * Note: The above follows WiringPi's numbering convention, just google it or something.
 */

#define SIN 17
#define SCLK 23
#define XLAT 27
#define BLANK 22

int tlcleds[NUM_TLCS * 24];
int xlat_needed = 0;
int tlcDone = 0;
struct timespec* sleepTime;

//ledIndex >= 0 && ledIndex < NUM_TLCS * 24
//value >= 0 && value < 4096
void tlcSetLed(int ledIndex, int value)
{
    if(ledIndex >= 0 && ledIndex < NUM_TLCS * 24 && value >= 0 && value < 4096)
        tlcleds[ledIndex] = value;
}

//value >= 0 && value < 4096
void tlcSetAllLeds(int value)
{
    if(value >= 0 && value < 4096)
        for(int i = 0; i < NUM_TLCS * 24; i++)
            tlcleds[i] = value;
}

void tlcClearLeds(void)
{
    tlcSetAllLeds(0);
}

//index >= 0 && index < NUM_TLCS * 24
int tlcGetLedValue(int index)
{
    if(index >= 0 && index < NUM_TLCS * 24)
        return index;
    return -1;
}

void delayStuff() 
{
    for(int i = 0; i < 100; i++)
        __asm__ ("nop");
}

void tlcPulsePin(int pin)
{
    digitalWrite(pin, 1);
    delayStuff();
    //nanosleep(sleepTime, NULL);
    digitalWrite(pin, 0);
}

void tlc5947init(void)
{
    char *buffer = malloc(sizeof(char)*60);
    sprintf(buffer, "gpio export %d out", SIN);
    if(system(buffer) == -1)
        printf("Wiring setup failed!\n");
    sprintf(buffer, "gpio export %d out", SCLK);
    if(system(buffer) == -1)
        printf("Wiring setup failed!\n");
    sprintf(buffer, "gpio export %d out", XLAT);
    if(system(buffer) == -1)
        printf("Wiring setup failed!\n");
    sprintf(buffer, "gpio export %d out", BLANK);
    if(system(buffer) == -1)
        printf("Wiring setup failed!\n");
    if(wiringPiSetupSys() == -1)
        printf("Wiring setup failed!\n");
    
    memset(&tlcleds, 0, sizeof(int));
    free(buffer);

    //pinMode(SIN, OUTPUT);
    //pinMode(SCLK, OUTPUT);
    //pinMode(XLAT, OUTPUT);
    //pinMode(BLANK, OUTPUT);


    sleepTime = malloc(sizeof(struct timespec));
    sleepTime->tv_sec = 0;
    sleepTime->tv_nsec = 5;
}

void tlc5947cleanup(void)
{
    tlcSetAllLeds(0);
    tlcUpdateLeds();

    tlcDone = 1;
}

void tlcUpdateLeds(void)
{
    for(int i = NUM_TLCS * 24 - 1; i >= 0; i--)
    {
        for(int j = 2048; j > 0; j /= 2)
        {
            digitalWrite(SIN, tlcleds[i] & j);
            delayStuff();
            tlcPulsePin(SCLK);
        }
    }
    digitalWrite(BLANK, 1);
    tlcPulsePin(XLAT);
    digitalWrite(BLANK, 0);
}
