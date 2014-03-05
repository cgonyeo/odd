#include "../odd.h"
#include "getAvgVolume.h"

//Sets the LEDs to a given color, and varies their strength with the average volume
void volumeAnimation1(double *params, double totalTime, odd_led_t *color, double *lastAvg)
{
    (void)params;
    (void)totalTime;
    
    //The average volume
    SAMPLE avg = getAvgVolume();

    //Dampening any reductions in strength by 50%, to reduce flickering
    if(avg < *lastAvg)
        avg = (avg + *lastAvg) / 2;

    //Set the LEDs
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * avg);
        setTempLED(i, 'g', color->G * avg);
        setTempLED(i, 'b', color->B * avg);
    }
    *lastAvg = avg;
}
