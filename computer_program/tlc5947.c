#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
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
 * 2(27) -> XLAT
 * 3(22) -> BLANK
 *
 * Note: The above follows WiringPi's numbering convention, just google it or something.
 */

#define XLAT 27
#define BLANK 22

typedef unsigned char byte;

int tlcleds[NUM_TLCS * 24];
byte bits[NUM_TLCS * 36];
int xlat_needed = 0;

int spiFile;

//Sets an LED to be a given value
//ledIndex >= 0 && ledIndex < NUM_TLCS * 24
//value >= 0 && value < 4096
void tlcSetLed(int ledIndex, int value)
{
    if(ledIndex >= 0 && ledIndex < NUM_TLCS * 24 && value >= 0 && value < 4096)
        tlcleds[ledIndex] = value;
}

//Sets all LEDs to the given value
//value >= 0 && value < 4096
void tlcSetAllLeds(int value)
{
    if(value >= 0 && value < 4096)
        for(int i = 0; i < NUM_TLCS * 24; i++)
            tlcleds[i] = value;
}

//Sets all channels to 0
void tlcClearLeds(void)
{
    tlcSetAllLeds(0);
}

//index >= 0 && index < NUM_TLCS * 24
//Returns the value of the requested LED, between 0 and 4095 (inclusive)
int tlcGetLedValue(int index)
{
    if(index >= 0 && index < NUM_TLCS * 24)
        return index;
    return -1;
}

//Do nothing 100 times
void delayStuff() 
{
    for(int i = 0; i < 100; i++)
        __asm__ ("nop");
}

//Pulses the given pin. Turns it on, and then off.
void tlcPulsePin(int pin)
{
    digitalWrite(pin, 1);
    delayStuff();
    digitalWrite(pin, 0);
}

//Makes some system calls to set up the GPIO pins,
//and initializes some other stuff
void tlc5947init(void)
{
    char *buffer = malloc(sizeof(char)*60);
    sprintf(buffer, "gpio export %d out", XLAT);
    if(system(buffer) == -1)
        printf("Wiring setup failed!\n");
    sprintf(buffer, "gpio export %d out", BLANK);
    if(system(buffer) == -1)
        printf("Wiring setup failed!\n");
    sprintf(buffer, "gpio load spi");
    if(system(buffer) == -1)
        printf("Wiring setup failed!\n");
    if(wiringPiSetupSys() == -1)
        printf("Wiring setup failed!\n");

    spiFile = wiringPiSPISetup(0, 15000000);
    if(spiFile == -1)
        perror("Error initializing spi things\n");
    
    memset(&tlcleds, 0, sizeof(int));
    free(buffer);
}

//Sets the LEDs to be off
void tlc5947cleanup(void)
{
    tlcSetAllLeds(0);
    tlcUpdateLeds();
}

//[2048,1024,512,256,128,64,32,16,8,4,2,1][2048,1024,512,256,128,64,32,16,8,4,2,1]
//[2048,1024,512,256,128,64,32,16][8,4,2,1,2048,1024,512,256][128,64,32,16,8,4,2,1] <- this but reversed
//We need to write 12 bits for each number, but ints are most definitely larger than that.
//To avoid writing a bunch of leading 0s for every number (which would break things),
//I'm going to fill the 12 least significant bits in to a byte (unsigned char) array, and then write that.
void tlcUpdateLeds(void)
{
    memset(bits, 0, sizeof(byte) * NUM_TLCS * 36);

    int bytecounter = 0;
    for(int i = (NUM_TLCS * 24) - 1; i >= 1; i -= 2)
    {   
        //fill the first byte
        for(int j = 2048; j >= 16; j /= 2)
        {
            bits[bytecounter] |= (tlcleds[i] & j) >> 4;
        }
        //fill the second byte
        bytecounter++;
        for(int j = 8; j >= 1; j /= 2)
        {
            bits[bytecounter] |= (tlcleds[i] & j) << 4;
        }
        for(int j = 2048; j >= 256; j/=2)
        {
            bits[bytecounter] |= (tlcleds[i - 1] & j) >> 8;
        }
        //fill the third byte
        bytecounter++;
        for(int j = 128; j >= 1; j/=2)
        {
            bits[bytecounter] |= tlcleds[i - 1] & j;
        }
        bytecounter++;
    }

    write(spiFile, bits, sizeof(byte) * NUM_TLCS * 36);

    digitalWrite(BLANK, 1);
    tlcPulsePin(XLAT);
    digitalWrite(BLANK, 0);
}
