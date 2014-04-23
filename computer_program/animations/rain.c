#include "../odd.h"

void rain(double *params, double totalTime, odd_led_t *color, double *storage)
{
    double frequency = params[0];
    double strength = params[1];
    double speed = params[2];

    double time = totalTime * frequency;

    //clear the tempLeds array so we can use it
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', 0);
        setTempLED(i, 'g', 0);
        setTempLED(i, 'b', 0);
    }

    double leds[NUM_LEDS];
    memset(leds, 0, sizeof(double) * NUM_LEDS);

    double numDrops = (int)speed;
    if(numDrops < speed)
        numDrops++;

    for(int i = 0; i < numDrops; i++) {
        srand((int)time - i);
        double locationDou = rand() * 1.0 / RAND_MAX;

        int location = locationDou * (NUM_LEDS - 1);
        double progress = (time - ((int)time - i)) / speed;

        leds[location] = 1 - progress;

        double wavePoint = locationDou + (0.1 * progress);
    }

    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * leds[i]);
        setTempLED(i, 'g', color->G * leds[i]);
        setTempLED(i, 'b', color->B * leds[i]);
    }
}
