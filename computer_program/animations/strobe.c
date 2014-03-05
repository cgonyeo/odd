#include "../odd.h"

//Animation: Turns the LEDs off and on and off and on and off and on and off...
void strobe(double *params, double totalTime, odd_led_t *color, double *storage)
{
    double speed = params[0];
    double time = totalTime * speed;
    double power = 0;
    if((int)time % 2 == 1)
        power = 1;
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * power);
        setTempLED(i, 'g', color->G * power);
        setTempLED(i, 'b', color->B * power);
    }
}
