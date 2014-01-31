#include "odd.h"

//Inverts all channels on the temp LEDS.
//For example:
//4095 -> 0
//0 -> 4095
void invertTempLeds()
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
        setTempLED(i, 'r', 4095 - getTempLED(i, 'r'));
        setTempLED(i, 'g', 4095 - getTempLED(i, 'g'));
        setTempLED(i, 'b', 4095 - getTempLED(i, 'b'));
	}
}

//Adds the current modifications to the LEDs to the led array
void addLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS])
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(getLED(i, 'r') + getTempLED(i, 'r') > 4095)
            setLED(i, 'r', 4095);
		else
            setLED(i, 'r', getLED(i, 'r') + getTempLED(i, 'r'));
		
		if(getLED(i, 'g') + getTempLED(i, 'g') > 4095)
            setLED(i, 'g', 4095);
		else
            setLED(i, 'g', getLED(i, 'g') + getTempLED(i, 'g'));
		
		if(getLED(i, 'b') + getTempLED(i, 'b') > 4095)
            setLED(i, 'b', 4095);
		else
            setLED(i, 'b', getLED(i, 'b') + getTempLED(i, 'b'));
	}
}

//Subtracts the current modifications to the LEDs from the led array
void subtractLeds(odd_led_t* leds[NUM_LEDS], odd_led_t* tempLeds[NUM_LEDS])
{
	for(int i = 0; i < NUM_LEDS; i++)
	{
		if(getLED(i, 'r') - getTempLED(i, 'r') < 0)
            setLED(i, 'r', 0);
		else
            setLED(i, 'r', getLED(i, 'r') - getTempLED(i, 'r'));
		
		if(getLED(i, 'g') - getTempLED(i, 'g') < 0)
            setLED(i, 'g', 0);
		else
            setLED(i, 'g', getLED(i, 'g') - getTempLED(i, 'g'));
		
		if(getLED(i, 'b') - getTempLED(i, 'b') < 0)
            setLED(i, 'b', 0);
		else
            setLED(i, 'b', getLED(i, 'b') - getTempLED(i, 'b'));
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
		if(getLED(i, 'r') * getTempLED(i, 'r') > 4095)
            setLED(i, 'r', 4095);
		else
            setLED(i, 'r', getLED(i, 'r') * getTempLED(i, 'r'));
		
		if(getLED(i, 'g') * getTempLED(i, 'g') > 4095)
            setLED(i, 'g', 4095);
		else
            setLED(i, 'g', getLED(i, 'g') * getTempLED(i, 'g'));
		
		if(getLED(i, 'b') * getTempLED(i, 'b') > 4095)
            setLED(i, 'b', 4095);
		else
            setLED(i, 'b', getLED(i, 'b') * getTempLED(i, 'b'));
	}
}
