#include "../odd.h"

//Animation: Sets all LEDs to color
void setAll(double *params, double totalTime, odd_led_t *color, double *storage)
{
    (void)params;
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setTempLED(i, 'r', color->R);
        setTempLED(i, 'g', color->G);
        setTempLED(i, 'b', color->B);
    }
}
