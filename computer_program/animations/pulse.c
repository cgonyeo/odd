#include "../odd.h"

//Animation: Like strobe but fades LEDs in and out and pauses when they're off.
void smoothStrobe(double *params, double totalTime, odd_led_t *color, double *storage)
{
    double speed = params[0];

    double time = totalTime * speed;
    double power = 0;
    if((int)time % 2 == 1)
        power = odd_remainder(time, 1);
    else
        power = 1 - odd_remainder(time, 1);
    power = odd_pow(power, 2);
    power *= 1.5;
    power -= 0.5;
    if(power < 0)
        power = 0;
    for(int i = 0; i < NUM_LEDS; i++) {
        setTempLED(i, 'r', color->R * power);
        setTempLED(i, 'g', color->G * power);
        setTempLED(i, 'b', color->B * power);
    }
}
