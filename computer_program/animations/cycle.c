#include "../odd.h"

//Animation: Lights up a clump of LEDs and then travels down the length
//of the strip. Wraps around. Can have multiple clumps, as per param #3
void cycle(double *params, double totalTime, odd_led_t *color, double *storage) {
    double speed = params[0];
    double radius = params[1];
    double num = params[2];

    //scale the time by our speed to alter the rate of tf the animation
    double time = totalTime * speed;
    //double to keep track of the location of the center
    double center = 0.0;
    //we want the center to go between 0 and NUM_LEDS - 1, 
    //due to 0 based indexing of leds.
    int numLeds = NUM_LEDS - 1;
    //clear the tempLeds array so we can use it
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', 0);
        setTempLED(i, 'g', 0);
        setTempLED(i, 'b', 0);
    }

    //Calculate the center
    center = odd_remainder(time, 1);
    if(center < 0) center += 1;
    center *= numLeds;
    
    for(int x = 0; x < num; x++)
    {
        double tempcenter = center + (x * 1.0 / num) * NUM_LEDS;
        if(tempcenter > NUM_LEDS) tempcenter -= NUM_LEDS;

        //Calculate the distance of each LED from the center
        //and do some math to figure out each LED's brightness from 0-1
        double ledDistances[NUM_LEDS];
        for(int i = 0; i < NUM_LEDS; i++) ledDistances[i] = 0;
        for(int i = tempcenter - radius; i <= tempcenter + radius; i++)
        {
            int index = i;
            if(index >= NUM_LEDS) index -= NUM_LEDS;
            if(index < 0) index += NUM_LEDS;

            if(tempcenter - i > 0)
                ledDistances[index] = tempcenter - i;
            else
                ledDistances[index] = i - tempcenter;
            ledDistances[index] = 
                    (radius - ledDistances[index]) / radius;
        }

        odd_led_t *color2 = color->next;
        //If there's only one color, set the LEDs
        //based on their brightnesses
        if(color2 == NULL)
        {
            for(int i = 0; i < NUM_LEDS; i++)
            {
                setTempLED(i, 'r', 
                        color->R * ledDistances[i] + getTempLED(i, 'r'));
                setTempLED(i, 'g', 
                        color->G * ledDistances[i] + getTempLED(i, 'g'));
                setTempLED(i, 'b', 
                        color->B * ledDistances[i] + getTempLED(i, 'b'));
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
                double colorStrength = (16/3) * 
                    (pow(((1.0 - ledDistances[i]) * 2) + 2, -2) 
                      - (1.0 / 15.5));
                setTempLED(i, 'r', 
                        colorTemp.R * colorStrength + getTempLED(i, 'r'));
                setTempLED(i, 'g', 
                        colorTemp.G * colorStrength + getTempLED(i, 'r'));
                setTempLED(i, 'b', 
                        colorTemp.B * colorStrength + getTempLED(i, 'r'));
            }
        }
    }
}
