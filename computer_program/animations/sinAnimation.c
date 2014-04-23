#include "../odd.h"

//Animation that makes the brightness of the LEDs follow a sin curve
void sinAnimation(double *params, double totalTime, odd_led_t *color, double *storage)
{
    double speed = params[0];

    double time = totalTime * speed;
    double power = 0;
    if((int)time % 2 == 1)
        power = fmod(time, 1);
    else
        power = 1 - fmod(time, 1);
    power = odd_sin(power);
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R * power);
        setTempLED(i, 'g', color->G * power);
        setTempLED(i, 'b', color->B * power);
    }
}
