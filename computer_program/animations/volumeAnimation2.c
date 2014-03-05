#include "../odd.h"
#include "getAvgVolume.h"

//Displays a volume meter, anchored to one side
void volumeAnimation2(double *params, double totalTime, odd_led_t *color, double *lastAvg)
{
    (void)params;
    (void)totalTime;
    
    //The average volume
    SAMPLE avg = getAvgVolume();

    //Dampening any reductions in strength by 50%, to reduce flickering
    if(avg < *lastAvg)
        avg = (avg + *lastAvg) / 2;

    //Set the LEDs
    for(int i = 0; i < avg * NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * avg);
        setTempLED(i, 'g', color->G * avg);
        setTempLED(i, 'b', color->B * avg);
    }
    for(int i = avg * NUM_LEDS; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', 0);
        setTempLED(i, 'g', 0);
        setTempLED(i, 'b', 0);
    }
    *lastAvg = avg;
}
