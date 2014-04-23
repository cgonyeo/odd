#include "../odd.h"

void wave(double *params, double totalTime, odd_led_t *color, double *storage) {
    double frequency = params[0];
    double strength = params[1];
    double speed = params[2];

    double time = totalTime * speed;

    //clear the tempLeds array so we can use it
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', 0);
        setTempLED(i, 'g', 0);
        setTempLED(i, 'b', 0);
    }

    double leds[NUM_LEDS];
    memset(leds, 0, sizeof(double) * NUM_LEDS);

    for(int x = 0; x < frequency; x++)
    {
        double center = (fmod(time, 1.0 / frequency) + x / frequency) / 2 
                            * (1.0 + (0.2 * strength));

        for(int i = 0; i < NUM_LEDS; i++)
        {
            double distanceToCenter = (center - i * 1.0 / NUM_LEDS) / strength;
            if(distanceToCenter > 0 && distanceToCenter < 0.1) {
                distanceToCenter *= 10;
                //double brightness = sin( -1.0 * distanceToCenter * PI - 1.5 * PI) / 2 + 0.5;
                double brightness = 0;
                if(distanceToCenter > 1.0/6.0)
                    brightness = sin(-1.0 * distanceToCenter * PI * 6.0/5 - 4.0/3 * PI) / 2 + 0.5;
                else
                    brightness = 6 * distanceToCenter;
                leds[i] += brightness;
                if(leds[i] > 1)
                    leds[i] = 1;
            }
        }
    }

    for(int i = 0; i < NUM_LEDS/2; i++) {
        setTempLED(NUM_LEDS / 2 - i - 1, 'r', leds[i] * color->R);
        setTempLED(NUM_LEDS / 2 - i - 1, 'g', leds[i] * color->G);
        setTempLED(NUM_LEDS / 2 - i - 1, 'b', leds[i] * color->B);
        setTempLED(NUM_LEDS / 2 + i,     'r', leds[i] * color->R);
        setTempLED(NUM_LEDS / 2 + i,     'g', leds[i] * color->G);
        setTempLED(NUM_LEDS / 2 + i,     'b', leds[i] * color->B);
    }
}
