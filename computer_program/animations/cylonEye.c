#include "../odd.h"

//Animation: Lights up a clump of LEDs and then travels back and forth across the row
//Follows a basic sin curve (moves faster in the middle, slower at the ends)
void cylonEye(double *params, double totalTime, odd_led_t *color, double *storage) {
    double speed = params[0];
    double radius = params[1];

    //scale the time by our speed to alter the rate of tf the animation
    double time = totalTime * speed;
    //double to keep track of the location of the center
    double center = 0.0;
    //we want the center to go between 0 and NUM_LEDS - 1, due to 0 based indexing of leds.
    int numLeds = NUM_LEDS - 1;
    //clear the tempLeds array so we can use it
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', 0);
        setTempLED(i, 'g', 0);
        setTempLED(i, 'b', 0);
    }
    
    //Calculate the center
    if((int)time % 2 == 1)
        center = odd_remainder(time, 1);
    else
        center = 1 - odd_remainder(time, 1);
    center = odd_sin(center);
    center *= numLeds;
    
    
    //Calculate the distance of each LED from the center, and do some math to figure out each LED's brightness
    double ledDistances[NUM_LEDS];
    for(int i = 0; i < NUM_LEDS; i++) ledDistances[i] = 0;
    for(int i = center - radius; i <= center + radius; i++)
    {
        if(i > 0 && i < NUM_LEDS)
        {
            if(center - i > 0)
                ledDistances[i] = center - i;
            else
                ledDistances[i] = i - center;
            ledDistances[i] = 
                    (radius - ledDistances[i]) / radius;
        }
    }

    odd_led_t *color2 = color->next;
    //If there's only one color, set the LEDs
    //based on their brightnesses
    if(color2 == NULL)
    {
        for(int i = 0; i < NUM_LEDS; i++)
        {
            setTempLED(i, 'r', color->R * ledDistances[i]);
            setTempLED(i, 'g', color->G * ledDistances[i]);
            setTempLED(i, 'b', color->B * ledDistances[i]);
        }
    }
    //Otherwise, let's have the colors fade together
    else
    {
        for(int i = 0; i < NUM_LEDS; i++)
        {
            if(ledDistances[i] != 0)
            {
                double colorDistance = ledDistances[i];
                odd_led_t colorTemp;
                colorTemp.R = color->R * colorDistance + 
                              color2->R * (1 / colorDistance);
                colorTemp.G = color->G * colorDistance + 
                              color2->G * (1 / colorDistance);
                colorTemp.B = color->B * colorDistance + 
                              color2->B * (1 / colorDistance);

                setTempLED(i, 'r', colorTemp.R * pow(colorDistance, 4));
                setTempLED(i, 'g', colorTemp.G * pow(colorDistance, 4));
                setTempLED(i, 'b', colorTemp.B * pow(colorDistance, 4));
            }
        }
    }
}
