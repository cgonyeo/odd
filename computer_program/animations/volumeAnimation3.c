#include "../odd.h"
#include "getAvgVolume.h"

//Displays a volume meter, centered on the display
void volumeAnimation3(double *params, double totalTime, odd_led_t *color, double *lastAvg)
{
    (void)params;
    (void)totalTime;
    
    SAMPLE avg = getAvgVolume();

    //Dampening any reductions in strength by 50%, to reduce flickering
    if(avg < *lastAvg)
        avg = (avg + *lastAvg) / 2;

    //Calculating the width of the volume meter, and the gap on either side
    int width = (int)(avg * NUM_LEDS);
    int gap = (int)((NUM_LEDS - width) / 2);

    //Setting the LEDs
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i,'r', 0);
        setTempLED(i,'g', 0);
        setTempLED(i,'b', 0);
    }
    for(int i = gap; i < gap+width; i++)
    {
        setTempLED(i,'r', color->R * avg);
        setTempLED(i,'g', color->G * avg);
        setTempLED(i,'b', color->B * avg);
    }
    *lastAvg = avg;
}
