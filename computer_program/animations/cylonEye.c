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
    for(int i = tempcenter - radius, i <= tempcenter + radius, i++)
    {
        if(i > 0 && i < NUM_LEDS)
        {
            if(tempcenter - i > 0)
                ledDistances[i] = tempcenter - i;
            else
                ledDistances[i] = i - tempcenter;
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
            double colorDistance = pow(ledDistances[i], 1);
            odd_led_t colorTemp;
            colorTemp.R = color->R * colorDistance + 
                          color2->R * (0.1 / colorDistance);
            colorTemp.G = color->G * colorDistance + 
                          color2->G * (0.1 / colorDistance);
            colorTemp.B = color->B * colorDistance + 
                          color2->B * (0.1 / colorDistance);

            //I have no clue how the fuck I came up 
            //with the following number
            double colorStrength = (16/3) * (pow(((1.0 - ledDistances[i]) * 2) + 2, -2) - (1.0 / 15.5));
            setTempLED(i, 'r', colorTemp.R * colorStrength);
            setTempLED(i, 'g', colorTemp.G * colorStrength);
            setTempLED(i, 'b', colorTemp.B * colorStrength);
            }
        }
    }
}
