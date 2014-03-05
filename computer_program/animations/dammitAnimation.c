#include "../odd.h"

//Displays a spectrograph (I think that's a word?) across the LEDs
//Each LED is mapped to a frequency, and it's strength varies with the strength in that frequency in the sound
void dammitAnimation(double *params, double totalTime, odd_led_t *color, double *storage)
{
    (void)params;

    SAMPLE soundBuffer[FFT_OUTPUT_SIZE];
    memset(soundBuffer, 0, sizeof(*soundBuffer) * FFT_OUTPUT_SIZE);
    runFFT(soundBuffer);

    SAMPLE ledSums[NUM_LEDS];

    memset(ledSums, 0, sizeof(*ledSums) * NUM_LEDS);

    //Mapping bins 0-80 to the leds logarithmically
    for(int i = 0; i < NUM_LEDS / 2; i++)
    {
        int low = pow(i + 1.0, 2) / pow(NUM_LEDS / 2.0, 2) * 160;
        int high = pow(i + 2.0, 2) / pow(NUM_LEDS / 2.0, 2) * 160;
        if(low == high)
            high++;

        for(int j = low; j < high; j++)
        {   
            ledSums[i] += soundBuffer[j];
        }
        ledSums[i] /= 1000;

        if(ledSums[i] > 1)
            printf("%f\n", ledSums[i]);

        ledSums[i] = pow(ledSums[i], 2);
        ledSums[i] *= (pow(i + 15, 2) / pow(NUM_LEDS + 15, 2)) * 66;
    }

    if(color->next == NULL)
        for(int i = 0; i < NUM_LEDS; i++)
        {
            setTempLED(i, 'r', ledSums[i] * color->R);
            setTempLED(i, 'g', ledSums[i] * color->G);
            setTempLED(i, 'b', ledSums[i] * color->B);
        }
    else
        for(int i = 0; i < NUM_LEDS / 2; i++)
        {
            float position = (i+0.0001) / (NUM_LEDS / 2);
            int redChannel   = color->R * position + color->next->R * (1 - position);
            int greenChannel = color->G * position + color->next->G * (1 - position);
            int blueChannel  = color->B * position + color->next->B * (1 - position);
            setTempLED(NUM_LEDS / 2 - i, 'r', ledSums[i] * redChannel);
            setTempLED(NUM_LEDS / 2 - i, 'g', ledSums[i] * greenChannel);
            setTempLED(NUM_LEDS / 2 - i, 'b', ledSums[i] * blueChannel);
            setTempLED(NUM_LEDS / 2 + i, 'r', ledSums[i] * redChannel);
            setTempLED(NUM_LEDS / 2 + i, 'g', ledSums[i] * greenChannel);
            setTempLED(NUM_LEDS / 2 + i, 'b', ledSums[i] * blueChannel);
        }
}
